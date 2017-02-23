/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
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
T2DInterpolator<T>::T2DInterpolator(const T2DDatafield<T>& image, PSplineKernel  kernel):
	m_coeff(image.get_size()), 
	m_kernel(kernel),
	m_x_boundary(produce_spline_boundary_condition("mirror")), 
	m_y_boundary(produce_spline_boundary_condition("mirror")),
	m_cache(kernel->size(), *m_x_boundary, *m_y_boundary)
{
	m_x_boundary->set_width(image.get_size().x); 
	m_y_boundary->set_width(image.get_size().y); 
	prefilter(image); 
}
template <typename T>
T2DInterpolator<T>::T2DInterpolator(const T2DDatafield<T>& image, PSplineKernel kernel, 
						      const CSplineBoundaryCondition& xbc, const CSplineBoundaryCondition& ybc):
	m_coeff(image.get_size()), 
	m_kernel(kernel),
	m_x_boundary(xbc.clone()), 
	m_y_boundary(ybc.clone()),
	m_cache(kernel->size(), *m_x_boundary, *m_y_boundary)
	
{
	prefilter(image); 
}

template <typename T>
void T2DInterpolator<T>::prefilter(const T2DDatafield<T>& image)
{
	m_x_boundary->set_width(image.get_size().x); 
	m_cache.x.reset();
	m_y_boundary->set_width(image.get_size().y); 
	m_cache.y.reset();
	


	min_max<typename T2DDatafield<T>::const_iterator >::get(image.begin(), image.end(), m_min, m_max);
	// always allow a zero value 
	if (T() < m_min) 
		m_min = T(); 

	
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
C2DWeightCache T2DInterpolator<T>::create_cache() const
{
	return C2DWeightCache(m_kernel->size(), *m_x_boundary, *m_y_boundary); 
}


template <typename T>
T2DInterpolator<T>::~T2DInterpolator()
{
}

template <typename T>
const typename T2DInterpolator<T>::TCoeff2D& T2DInterpolator<T>::get_coefficients() const
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
	
	static typename C::value_type apply(const C&  coeff, const C2DWeightCache& m_cache) 
	{
		U result = U();
		for (size_t y = 0; y < size; ++y) {
			U rx = U();
			const U *p = &coeff(0, m_cache.y.index[y]);
			for (size_t x = 0; x < size; ++x) {
				rx += m_cache.x.weights[x] * p[m_cache.x.index[x]];
			}
			result += m_cache.y.weights[y] * rx; 
		}
		return result; 
	}
};


template <typename T>
typename T2DInterpolator<T>::TCoeff2D::value_type T2DInterpolator<T>::evaluate() const
{
	typedef typename TCoeff2D::value_type U; 

	U result = U();
	
	// give the compiler some chance to optimize and unroll the 
	// interpolation loop by creating some fixed size calls  
	switch (m_kernel->size()) {
	case 2: result = add_2d<TCoeff2D,2>::apply(m_coeff, m_cache); break; 
	case 3: result = add_2d<TCoeff2D,3>::apply(m_coeff, m_cache); break; 
	case 4: result = add_2d<TCoeff2D,4>::apply(m_coeff, m_cache); break; 
	case 5: result = add_2d<TCoeff2D,5>::apply(m_coeff, m_cache); break; 
	case 6: result = add_2d<TCoeff2D,6>::apply(m_coeff, m_cache); break; 
	default: {
		/* perform interpolation */
		for (size_t y = 0; y < m_kernel->size(); ++y) {
			U rx = U();
			const typename  TCoeff2D::value_type *p = &m_coeff(0, m_cache.y.index[y]);
			for (size_t x = 0; x < m_kernel->size(); ++x) {
				rx += m_cache.x.weights[x] * p[m_cache.x.index[x]];
				cvdebug() << m_cache.x.weights[x] << "*" << p[m_cache.x.index[x]] << "\n"; 
			}
			result += m_cache.y.weights[y] * rx; 
		}
	}
	} // end switch 

	return result; 
}

template <class C, int size>
struct add_2d_new {
	typedef typename C::value_type U; 
	
	static typename C::value_type value(const C&  coeff, const C2DWeightCache& cache) 
	{
		U result = U();
		if (cache.x.is_flat) {
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, cache.y.index[y]);
				for (size_t x = 0; x < size; ++x) {
					rx += cache.x.weights[x] * p[cache.x.start_idx + x];
				}
				result += cache.y.weights[y] * rx; 
			}
		}else{
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, cache.y.index[y]);
				for (size_t x = 0; x < size; ++x) {
					rx += cache.x.weights[x] * p[cache.x.index[x]];
				}
				result += cache.y.weights[y] * rx; 
			}
		}
		return result; 
	}
};

template <typename T>
struct add_2d_new<T2DDatafield< T >, 1> {
	

	static T value(const T2DDatafield< T >&  coeff, 
		       const C2DWeightCache& cache) {
		return coeff(cache.x.index[0], cache.y.index[0]); 
	}
}; 


#ifdef __SSE2__
template <>
struct add_2d_new<T2DDatafield< double >, 4> {
	static double value(const T2DDatafield< double >&  coeff, 
			    const C2DWeightCache& cache); 
}; 
#endif

#ifdef __SSE__

template <>
struct add_2d_new<T2DDatafield< float >, 4> {
	

	static float value(const T2DDatafield< float >&  coeff, 
			   const C2DWeightCache& cache); 
}; 
#endif


template <typename T>
T  T2DInterpolator<T>::operator () (const C2DFVector& x, C2DWeightCache& cache) const
{
	typedef typename TCoeff2D::value_type U; 
	
	// x will usually be the fastest changing index, therefore, it is of no use to use the cache 
	// at the same time it's access may be handled "flat" 
	m_kernel->get_uncached(x.x, cache.x);

	// the other two coordinates are changing slowly and caching makes sense 
	// however, the index set will always be fully evaluated 
	if (x.y != cache.y.x) 
		m_kernel->get_cached(x.y, cache.y);
	
	U result = U();
	// now we give the compiler a chance to optimize based on kernel size and data type.  
	// Some of these call also use template specialization to provide an optimized code path.  
	// With SSE and SSE2 available kernel sizes 2 and 4 and the use of float and double 
	// scalar fields are optimized.
	switch (m_kernel->size()) {
	case 1: result = add_2d_new<TCoeff2D,1>::value(m_coeff, cache); break; 
	case 2: result = add_2d_new<TCoeff2D,2>::value(m_coeff, cache); break; 
	case 3: result = add_2d_new<TCoeff2D,3>::value(m_coeff, cache); break; 
	case 4: result = add_2d_new<TCoeff2D,4>::value(m_coeff, cache); break; 
	case 5: result = add_2d_new<TCoeff2D,5>::value(m_coeff, cache); break; 
	case 6: result = add_2d_new<TCoeff2D,6>::value(m_coeff, cache); break; 
	default: {
		assert(0 && "kernel sizes above 5 are not implemented"); 
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}


template <typename T>
T  T2DInterpolator<T>::operator () (const C2DFVector& x) const
{
	typedef typename TCoeff2D::value_type U;
	
	CScopedLock lock(m_cache_lock); 
	
	m_kernel->get_uncached(x.x, m_cache.x);
	
	if (x.y != m_cache.y.x) 
		m_kernel->get_cached(x.y, m_cache.y);
	
	U result = U();
	
	// give the compiler some chance to optimize 
	switch (m_kernel->size()) {
	case 1: result = add_2d_new<TCoeff2D,1>::value(m_coeff, m_cache); break; 
	case 2: result = add_2d_new<TCoeff2D,2>::value(m_coeff, m_cache); break; 
	case 3: result = add_2d_new<TCoeff2D,3>::value(m_coeff, m_cache); break; 
	case 4: result = add_2d_new<TCoeff2D,4>::value(m_coeff, m_cache); break; 
	case 5: result = add_2d_new<TCoeff2D,5>::value(m_coeff, m_cache); break; 
	case 6: result = add_2d_new<TCoeff2D,6>::value(m_coeff, m_cache); break; 
	default: {
		/* perform interpolation */
		assert(0 && "spline degree > 5 not implemented");
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

template <typename T>
T2DVector<T> T2DInterpolator<T>::derivative_at(const C2DFVector& x) const
{
	T2DVector<T> result;
	CScopedLock lock(m_cache_lock); 
	
	// cut at boundary maybe we can do better
	if (x.x < 0.0 || x.y < 0.0 || x.x >= m_coeff.get_size().x || x.y >= m_coeff.get_size().y)
		return result;

	const int xi = m_kernel->get_indices(x.x, m_cache.x.index); 
	const double fx = x.x - xi; 
	m_kernel->get_derivative_weights(fx, m_cache.x.weights); 
	m_x_boundary->apply(m_cache.x.index, m_cache.x.weights);
	
	const int yi = m_kernel->get_indices(x.y, m_cache.y.index); 
	const double fy = x.y - yi; 
	m_kernel->get_weights(fy, m_cache.y.weights);
	m_y_boundary->apply(m_cache.y.index, m_cache.y.weights);
	

	typename TCoeff2D::value_type r = evaluate();	
	result.x = round_to<typename TCoeff2D::value_type, T>::value(r);  

	
	m_kernel->get_weights(fx, m_cache.x.weights); 
	m_kernel->get_derivative_weights(fy, m_cache.y.weights); 

	r = evaluate();	
	result.y = round_to<typename TCoeff2D::value_type, T>::value(r);  
	
	return result; 
}

NS_MIA_END



