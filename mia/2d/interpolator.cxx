/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
  The filter routines for splines and omoms is based on code by 
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/
*/

#include <cmath>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

NS_MIA_BEGIN

template <typename T>
struct __dispatch_min_max<T2DVector<T> > {
	static void apply(const T2DVector<T>& i, T2DVector<T>& min, T2DVector<T>& max) {
		if (i.x > max.x) max.x = i.x; 
		if (i.y > max.y) max.y = i.y; 
		if (i.x < min.x) min.x = i.x; 
		if (i.y < min.y) min.y = i.y; 
	}
}; 


struct FConvertVector {
	C2DDVector operator () (const C2DFVector& x) const {
		return C2DDVector(x); 
	}
}; 

template <> 
struct __dispatch_copy<std::vector<C2DFVector>, std::vector<C2DDVector> > {
	static void apply(const std::vector<C2DFVector>& input, std::vector<C2DDVector>& output){
		std::transform(input.begin(), input.end(), output.begin(), FConvertVector());
	}
}; 

template <> 
struct __dispatch_copy<C2DFVectorfield, C2DDVectorfield > {
	static void apply(const C2DFVectorfield& input, C2DDVectorfield& output){
		std::transform(input.begin(), input.end(), output.begin(), FConvertVector());
	}
}; 


template <typename T>
T2DConvoluteInterpolator<T>::T2DConvoluteInterpolator(const T2DDatafield<T>& image, PSplineKernel  kernel):
	m_coeff(image.get_size()), 
	m_kernel(kernel),
	m_x_boundary(produce_spline_boundary_condition("mirror", image.get_size().x)), 
	m_y_boundary(produce_spline_boundary_condition("mirror", image.get_size().y)), 
	m_x_index(kernel->size()),
	m_y_index(kernel->size()),
	m_x_weight(kernel->size()),
	m_y_weight(kernel->size()), 
	m_x_cache(kernel->size(), *m_x_boundary, false), 
	m_y_cache(kernel->size(), *m_y_boundary, true)
{
	prefilter(image); 
}
template <typename T>
T2DConvoluteInterpolator<T>::T2DConvoluteInterpolator(const T2DDatafield<T>& image, PSplineKernel kernel, 
						      const CSplineBoundaryCondition& xbc, const CSplineBoundaryCondition& ybc):
	m_coeff(image.get_size()), 
	m_kernel(kernel),
	m_x_boundary(xbc.clone()), 
	m_y_boundary(ybc.clone()), 
	m_x_index(kernel->size()),
	m_y_index(kernel->size()),
	m_x_weight(kernel->size()),
	m_y_weight(kernel->size()), 
	m_x_cache(kernel->size(), *m_x_boundary, false), 
	m_y_cache(kernel->size(), *m_y_boundary, true)
{
	prefilter(image); 
}

template <typename T>
void T2DConvoluteInterpolator<T>::prefilter(const T2DDatafield<T>& image)
{
	m_x_boundary->set_width(image.get_size().x); 
	m_x_cache.reset();
	m_y_boundary->set_width(image.get_size().y); 
	m_y_cache.reset();
	


	min_max<typename T2DDatafield<T>::const_iterator >::get(image.begin(), image.end(), m_min, m_max);
	
	// copy the data
	__dispatch_copy<T2DDatafield<T>, TCoeff2D >::apply(image, m_coeff); 
	if (m_kernel->get_poles().empty()) 
		return; 

	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	
	{
		coeff_vector buffer(cachXSize);
		for (int y = 0; y < cachYSize; y++) {
			m_coeff.get_data_line_x(y,buffer);
			m_x_boundary->filter_line(buffer, m_kernel->get_poles());
			m_coeff.put_data_line_x(y,buffer);
		}
	}
	
	{
		coeff_vector buffer(cachYSize);
		for (int x = 0; x < cachXSize; x++) {
			m_coeff.get_data_line_y(x,buffer);
			m_y_boundary->filter_line(buffer, m_kernel->get_poles());
			m_coeff.put_data_line_y(x,buffer);
		}
	}
}

template <typename T>
T2DConvoluteInterpolator<T>::~T2DConvoluteInterpolator()
{
}

template <typename T>
const typename T2DConvoluteInterpolator<T>::TCoeff2D& T2DConvoluteInterpolator<T>::get_coefficients() const
{
	return m_coeff; 
}
	

template <class T, class U>
struct bounded<T2DVector<T>, T2DVector<U> > {
	static void apply(T2DVector<T>& r, const T2DVector<U>& min, const T2DVector<U>& max)
	{
		r.x = (r.x >= min.x) ? ( (r.x <= max.x) ? r.x : max.x) : min.x;
		r.y = (r.y >= min.y) ? ( (r.y <= max.y) ? r.y : max.y) : min.y;
	}
};

// this struct is created to letthe compiler create fixed sized loops 
// which can then be optimezed and unrolled 
template <class C, int size>
struct add_2d {
	typedef typename C::value_type U; 
	
	static typename C::value_type apply(const C&  coeff, const CSplineKernel::VWeight& xweight, 
					    const CSplineKernel::VWeight& yweight,
					    const CSplineKernel::VIndex& xindex, 
					    const CSplineKernel::VIndex& yindex) 
	{
		U result = U();
		for (size_t y = 0; y < size; ++y) {
			U rx = U();
			const U *p = &coeff(0, yindex[y]);
			for (size_t x = 0; x < size; ++x) {
				rx += xweight[x] * p[xindex[x]];
			}
			result += yweight[y] * rx; 
		}
		return result; 
	}
};


template <typename T>
typename T2DConvoluteInterpolator<T>::TCoeff2D::value_type T2DConvoluteInterpolator<T>::evaluate() const
{
	typedef typename TCoeff2D::value_type U; 

	U result = U();
	
	// give the compiler some chance to optimize and unroll the 
	// interpolation loop by creating some fixed size calls  
	switch (m_kernel->size()) {
	case 2: result = add_2d<TCoeff2D,2>::apply(m_coeff, m_x_weight, m_y_weight, 
					    m_x_index, m_y_index); break; 
	case 3: result = add_2d<TCoeff2D,3>::apply(m_coeff, m_x_weight, m_y_weight, 
					    m_x_index, m_y_index); break; 
	case 4: result = add_2d<TCoeff2D,4>::apply(m_coeff, m_x_weight, m_y_weight, 
					    m_x_index, m_y_index); break; 
	case 5: result = add_2d<TCoeff2D,5>::apply(m_coeff, m_x_weight, m_y_weight, 
					    m_x_index, m_y_index); break; 
	case 6: result = add_2d<TCoeff2D,6>::apply(m_coeff, m_x_weight, m_y_weight, 
					    m_x_index, m_y_index); break; 
	default: {
		/* perform interpolation */
		for (size_t y = 0; y < m_kernel->size(); ++y) {
			U rx = U();
			const typename  TCoeff2D::value_type *p = &m_coeff(0, m_y_index[y]);
			for (size_t x = 0; x < m_kernel->size(); ++x) {
				rx += m_x_weight[x] * p[m_x_index[x]];
				cvdebug() << m_x_weight[x] << "*" << p[m_x_index[x]] << "\n"; 
			}
			result += m_y_weight[y] * rx; 
		}
	}
	} // end switch 

	return result; 
}

template <class C, int size>
struct add_2d_new {
	typedef typename C::value_type U; 
	
	static typename C::value_type value(const C&  coeff, const CSplineKernel::SCache& xc, 
					    const CSplineKernel::SCache& yc) 
	{
		U result = U();
		if (xc.is_flat) {
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, yc.index[y]);
				for (size_t x = 0; x < size; ++x) {
					rx += xc.weights[x] * p[xc.start_idx + x];
				}
				result += yc.weights[y] * rx; 
			}
		}else{
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, yc.index[y]);
				for (size_t x = 0; x < size; ++x) {
					rx += xc.weights[x] * p[xc.index[x]];
				}
				result += yc.weights[y] * rx; 
			}
		}
		return result; 
	}
};

template <typename T>
struct add_2d_new<T2DDatafield< T >, 1> {
	

	static T value(const T2DDatafield< T >&  coeff, 
		       const CSplineKernel::SCache& xc, 
		       const CSplineKernel::SCache& yc) {
		return coeff(xc.index[0], yc.index[0]); 
	}
}; 


#ifdef __SSE__
template <>
struct add_2d_new<T2DDatafield< double >, 4> {
	static double value(const T2DDatafield< double >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc); 
}; 
#endif

#ifdef __SSE2__

template <>
struct add_2d_new<T2DDatafield< float >, 4> {
	

	static float value(const T2DDatafield< float >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc); 
}; 
#endif


template <typename T>
T  T2DConvoluteInterpolator<T>::operator () (const C2DFVector& x) const
{
	typedef typename TCoeff2D::value_type U; 
	
	m_kernel->get_uncached(x.x, m_x_cache);
	
	if (x.y != m_y_cache.x) 
		m_kernel->get_cached(x.y, m_y_cache);
	
	U result = U();
	
	// give the compiler some chance to optimize 
	switch (m_kernel->size()) {
	case 1: result = add_2d_new<TCoeff2D,1>::value(m_coeff, m_x_cache, m_y_cache); break; 
	case 2: result = add_2d_new<TCoeff2D,2>::value(m_coeff, m_x_cache, m_y_cache); break; 
	case 3: result = add_2d_new<TCoeff2D,3>::value(m_coeff, m_x_cache, m_y_cache); break; 
	case 4: result = add_2d_new<TCoeff2D,4>::value(m_coeff, m_x_cache, m_y_cache); break; 
	case 5: result = add_2d_new<TCoeff2D,5>::value(m_coeff, m_x_cache, m_y_cache); break; 
	case 6: result = add_2d_new<TCoeff2D,6>::value(m_coeff, m_x_cache, m_y_cache); break; 
	default: {
		/* perform interpolation */
		assert(0 && "spline degree > 5 not implemented");
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

template <typename T>
T2DVector<T> T2DConvoluteInterpolator<T>::derivative_at(const C2DFVector& x) const
{
	T2DVector<T> result;
	
	// cut at boundary maybe we can do better
	if (x.x < 0.0 || x.y < 0.0 || x.x >= m_coeff.get_size().x || x.y >= m_coeff.get_size().y)
		return result;

	const int xi = m_kernel->get_indices(x.x, m_x_index); 
	const double fx = x.x - xi; 
	m_kernel->get_derivative_weights(fx, m_x_weight); 
	m_x_boundary->apply(m_x_index, m_x_weight);
	
	const int yi = m_kernel->get_indices(x.y, m_y_index); 
	const double fy = x.y - yi; 
	m_kernel->get_weights(fy, m_y_weight);
	m_y_boundary->apply(m_y_index, m_y_weight);
	

	typename TCoeff2D::value_type r = evaluate();	
	result.x = round_to<typename TCoeff2D::value_type, T>::value(r);  

	
	m_kernel->get_weights(fx, m_x_weight); 
	m_kernel->get_derivative_weights(fy, m_y_weight); 

	r = evaluate();	
	result.y = round_to<typename TCoeff2D::value_type, T>::value(r);  
	
	return result; 
}

NS_MIA_END



