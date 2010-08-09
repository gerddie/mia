/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *

  The filter routines for splines and omoms is based on code by 
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/
   
*/

#include <cmath>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

NS_MIA_BEGIN

template <typename T>
T2DDirectInterpolator<T>::T2DDirectInterpolator(const T2DDatafield<T>& data):
       _M_data(data)
{
}

template <typename T>
T2DNNInterpolator<T>::T2DNNInterpolator(const T2DDatafield<T>& image):
	T2DDirectInterpolator<T>(image)
{
}

template <typename T>
T
T2DNNInterpolator<T>::operator () (const C2DFVector& x)const
{
	C2DBounds ix( (unsigned int)(x.x + 0.5f), (unsigned int)(x.y + 0.5f));
	if (ix.x < this->data().get_size().x && 
	    ix.y < this->data().get_size().y )
		return this->data()(ix);
	else
		return T();
}

template <typename T>
T2DVector<T> T2DNNInterpolator<T>::derivative_at(const C2DFVector& /*x*/) const
{
	throw std::invalid_argument("NN interpolator doesn't support a derivative"); 
}

template <typename T>
T2DBilinearInterpolator<T>::T2DBilinearInterpolator(const T2DDatafield<T>& image):
	T2DDirectInterpolator<T>(image), 
	_M_size(image.get_size()),
	_M_sizeb(image.get_size())
		
{
	_M_sizeb.x -= 1.0f;
	_M_sizeb.y -= 1.0f; 
}

template <typename T> 
struct bilin_dispatch {
	static T apply(const T2DDatafield<T>& data, const C2DFVector& p, const C2DFVector& sizeb) {
	// Calculate the coordinates and the distances
		const float x  = floorf(p.x);
		const float fx = p.x - x;
		const float dx = 1.0f - fx;
		
		const float y = floorf(p.y);
		const float fy = p.y - y;
		const float  dy = 1.0f - fy;
		
		if ( x < sizeb.x && y  < sizeb.y && x > 0.0 && y > 0.0 ) {
			const unsigned int ux = data.get_size().x;
			const unsigned int uy = (unsigned int)(x + ux *  y); 
			const T *ptr = &data[uy];
			const T *ptr_h = &ptr[ux];
			return T ( dy * (dx * ptr[0]    + fx * ptr[1]) + 
				      fy * (dx * ptr_h[0]  + fx * ptr_h[1]));   
		} else {
			const unsigned int ux = (unsigned int)x, uy = (unsigned int)y;
			const T  a1 = T(dx * data(ux  , uy    ) + fx * data(ux+1, uy    ));
			const T  a3 = T(dx * data(ux  , uy+1  ) + fx * data(ux+1, uy+1  ));
			return T(dy * a1 + fy * a3);
		}
		
	}
};

// for booleans we go the slow way ...
template <> 
struct bilin_dispatch<bool> {
	static bool apply(const T2DDatafield<bool>& data, const C2DFVector& p, const C2DFVector& /*sizeb*/) {
		const float x  = floorf(p.x);
		const float fx = p.x - x;
		const float dx = 1.0f - fx;
		
		const float y = floorf(p.y);
		const float fy = p.y - y;
		const float  dy = 1.0f - fy;
		
		
		const size_t ux = (size_t)x, uy = (size_t)y;  
		const float  a1 = dx * data(ux  , uy  ) + fx * data(ux+1, uy  );
		const float  a3 = dx * data(ux  , uy+1) + fx * data(ux+1, uy+1);
		return  dy * a1 + fy * a3 > 0.5;
		
	}
};
	
template <typename T>
T  T2DBilinearInterpolator<T>::operator () (const C2DFVector& p)const
{
	return bilin_dispatch<T>::apply(this->data(), p, this->_M_sizeb); 
}

template <typename T>
T2DVector<T> T2DBilinearInterpolator<T>::derivative_at(const C2DFVector& /*x*/) const
{
	assert(!"not yet implemented"); 
}


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
T2DConvoluteInterpolator<T>::T2DConvoluteInterpolator(const T2DDatafield<T>& image, SHARED_PTR(CBSplineKernel)  kernel):
	_M_coeff(image.get_size()), 
	_M_size2(image.get_size() + image.get_size() - C2DBounds(2,2)),
	_M_kernel(kernel),
	_M_x_index(kernel->size()),
	_M_y_index(kernel->size()),
	_M_x_weight(kernel->size()),
	_M_y_weight(kernel->size())
{
	min_max<typename T2DDatafield<T>::const_iterator >::get(image.begin(), image.end(), _M_min, _M_max);
	
	// copy the data
	__dispatch_copy<T2DDatafield<T>, TCoeff2D >::apply(image, _M_coeff); 
	
	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	
	const std::vector<double>& poles = _M_kernel->get_poles();

	{
		coeff_vector buffer(cachXSize);
		for (int y = 0; y < cachYSize; y++) {
			_M_coeff.get_data_line_x(y,buffer);
			filter_line(buffer, poles);
			_M_coeff.put_data_line_x(y,buffer);
		}
	}
	
	{
		coeff_vector buffer(cachYSize);
		for (int x = 0; x < cachXSize; x++) {
			_M_coeff.get_data_line_y(x,buffer);
			filter_line(buffer, poles);
			_M_coeff.put_data_line_y(x,buffer);
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
	return _M_coeff; 
}
	
template <typename T>
void T2DConvoluteInterpolator<T>::filter_line(coeff_vector& coeff, 
					 const std::vector<double>& poles)
{
	/* special case required by mirror boundaries */
	if (coeff.size() < 2) {
		return;
	}
	/* compute the overall gain */
	double	lambda = 1.0;
	for (size_t k = 0; k < poles.size() ; ++k) {
		lambda  *=  2 - poles[k] - 1.0 / poles[k];
	}
	
	/* apply the gain */
	for_each(coeff.begin(), coeff.end(), 
		 FMultBy<typename coeff_vector::value_type>(lambda));
	
	/* loop over all poles */
	for (size_t k = 0; k < poles.size(); ++k) {
		/* causal initialization */
		coeff[0] = initial_coeff(coeff, poles[k]);
		
		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			coeff[n] += poles[k] * coeff[n - 1];
		}
		
		/* anticausal initialization */
		coeff[coeff.size() - 1] = initial_anti_coeff(coeff, poles[k]);
		/* anticausal recursion */
		for (int n = coeff.size() - 2; 0 <= n; n--) {
			coeff[n] = poles[k] * (coeff[n + 1] - coeff[n]);
		}
	}
}

template <typename T>
typename T2DConvoluteInterpolator<T>::TCoeff2D::value_type  
T2DConvoluteInterpolator<T>::initial_coeff(const coeff_vector& coeff, 
					   double pole)
{
	/* full loop */
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	typename T2DConvoluteInterpolator<T>::TCoeff2D::value_type  
		sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n <= coeff.size()  - 2L; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	return(sum / (1.0 - zn * zn));
}

template <typename T>
typename T2DConvoluteInterpolator<T>::TCoeff2D::value_type 
T2DConvoluteInterpolator<T>::initial_anti_coeff(const coeff_vector& coeff, 
						double pole)
{
	return((pole / (pole * pole - 1.0)) * (pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
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
	
	static typename C::value_type apply(const C&  coeff, const std::vector<double>& xweight, 
					    const std::vector<double>& yweight,
					    const std::vector<int>& xindex, 
					    const std::vector<int>& yindex) 
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

#if 0 //ned(__SSE2__) 

#ifdef __GNUC__
#define _mm_loadu_pd    __builtin_ia32_loadupd
#define _mm_storeu_pd   __builtin_ia32_storeupd
#define _mm_add_pd      __builtin_ia32_addpd
#define _mm_mul_pd      __builtin_ia32_mulpd
#define _mm_unpacklo_pd __builtin_ia32_unpcklpd
#define _mm_unpackhi_pd __builtin_ia32_unpckhpd

#endif

template <>
struct add_2d<T2DDatafield<double>,4> {
	static double apply(const T2DDatafield<double>&  coeff, 
			    const std::vector<double>& xweight, 
			    const std::vector<double>& yweight,
			    const std::vector<int>& xindex, 
			    const std::vector<int>& yindex) 
	{
		typedef double v2df __attribute__ ((vector_size (16)));
		
		double wx[4] __attribute__((aligned(16))); 
		double wy[4] __attribute__((aligned(16))); 
		copy(xweight.begin(), xweight.end(), wx); 
		copy(yweight.begin(), yweight.end(), wy); 
		
		const double *pp0 = &coeff(0, yindex[0]);
		const double *pp1 = &coeff(0, yindex[1]);
		const double *pp2 = &coeff(0, yindex[2]);
		const double *pp3 = &coeff(0, yindex[3]);
		
		const double p0[4] __attribute__((aligned(16)))
			=  {pp0[xindex[0]], pp0[xindex[1]], pp0[xindex[2]], pp0[xindex[3]]}; 
		const double p1[4] __attribute__((aligned(16)))
			=  {pp1[xindex[0]], pp1[xindex[1]], pp1[xindex[2]], pp1[xindex[3]]}; 
		const double p2[4] __attribute__((aligned(16))) 
			=  {pp2[xindex[0]], pp2[xindex[1]], pp2[xindex[2]], pp2[xindex[3]]}; 
		const double p3[4] __attribute__((aligned(16)))  
			 = {pp3[xindex[0]], pp3[xindex[1]], pp3[xindex[2]], pp3[xindex[3]]}; 
		

		const register v2df rwx01 = _mm_loadu_pd(&wx[0]); 
		const register v2df rwx23 = _mm_loadu_pd(&wx[2]); 
		
		const register v2df rwy01 = _mm_loadu_pd(&wy[0]); 
		const register v2df rwy23 = _mm_loadu_pd(&wy[2]); 
		
		register v2df rp001 =_mm_mul_pd(rwx01, &p0[0]); 
		register v2df rp023 =_mm_mul_pd(rwx23, &p0[2]); 
		register v2df rp101 =_mm_mul_pd(rwx01, &p1[0]); 
		register v2df rp123 =_mm_mul_pd(rwx23, &p1[2]); 
		register v2df rp201 =_mm_mul_pd(rwx01, &p2[0]); 
		register v2df rp223 =_mm_mul_pd(rwx23, &p2[2]); 
		register v2df rp301 =_mm_mul_pd(rwx01, &p3[0]); 
		register v2df rp323 =_mm_mul_pd(rwx23, &p3[2]); 
		
		register v2df rs01_0 = _mm_unpacklo_pd(rp001, rp101); 
		register v2df rs23_0 = _mm_unpacklo_pd(rp201, rp301); 
		register v2df rs01_1 = _mm_unpackhi_pd(rp001, rp101); 
		register v2df rs23_1 = _mm_unpackhi_pd(rp201, rp301); 

		register v2df rs01_2 = _mm_unpacklo_pd(rp023, rp123); 
		register v2df rs23_2 = _mm_unpacklo_pd(rp223, rp323); 
		register v2df rs01_3 = _mm_unpackhi_pd(rp023, rp123); 
		register v2df rs23_3 = _mm_unpackhi_pd(rp223, rp323); 

		
		register v2df s01_01 = _mm_add_pd(rs01_0, rs01_1); 
		register v2df s01_23 = _mm_add_pd(rs01_2, rs01_3); 

		register v2df s23_01 = _mm_add_pd(rs23_0, rs23_1); 
		register v2df s23_23 = _mm_add_pd(rs23_2, rs23_3); 

		register v2df s01_0123 =  _mm_add_pd(s01_01, s01_23); 
		register v2df s23_0123 =  _mm_add_pd(s23_01, s23_23); 
		
		s01_0123 = _mm_mul_pd(rwy01, s01_0123); 
		s23_0123 = _mm_mul_pd(rwy23, s23_0123); 
		
		register v2df sum2 = _mm_add_pd(s01_0123, s23_0123); 
		
		double s[2] __attribute__((aligned(16))); 
		_mm_storeu_pd(s, sum2); 
		return s[0] + s[1]; 
	}
};

#endif // __SSE2__

template <typename T>
typename T2DConvoluteInterpolator<T>::TCoeff2D::value_type T2DConvoluteInterpolator<T>::evaluate() const
{
	typedef typename TCoeff2D::value_type U; 

	U result = U();
	
	// give the compiler some chance to optimize and unroll the 
	// interpolation loop by creating some fixed size calls  
	switch (_M_kernel->size()) {
	case 2: result = add_2d<TCoeff2D,2>::apply(_M_coeff, _M_x_weight, _M_y_weight, 
					    _M_x_index, _M_y_index); break; 
	case 3: result = add_2d<TCoeff2D,3>::apply(_M_coeff, _M_x_weight, _M_y_weight, 
					    _M_x_index, _M_y_index); break; 
	case 4: result = add_2d<TCoeff2D,4>::apply(_M_coeff, _M_x_weight, _M_y_weight, 
					    _M_x_index, _M_y_index); break; 
	case 5: result = add_2d<TCoeff2D,5>::apply(_M_coeff, _M_x_weight, _M_y_weight, 
					    _M_x_index, _M_y_index); break; 
	case 6: result = add_2d<TCoeff2D,6>::apply(_M_coeff, _M_x_weight, _M_y_weight, 
					    _M_x_index, _M_y_index); break; 
	default: {
		/* perform interpolation */
		for (size_t y = 0; y < _M_kernel->size(); ++y) {
			U rx = U();
			const typename  TCoeff2D::value_type *p = &_M_coeff(0, _M_y_index[y]);
			for (size_t x = 0; x < _M_kernel->size(); ++x) {
				rx += _M_x_weight[x] * p[_M_x_index[x]];
				cvdebug() << _M_x_weight[x] << "*" << p[_M_x_index[x]] << "\n"; 
			}
			result += _M_y_weight[y] * rx; 
		}
	}
	} // end switch 

	return result; 
}

template <typename T>
T  T2DConvoluteInterpolator<T>::operator () (const C2DFVector& x) const
{
	typedef typename TCoeff2D::value_type U; 
	

	(*_M_kernel)(x.x, _M_x_weight, _M_x_index);
	(*_M_kernel)(x.y, _M_y_weight, _M_y_index);

	
	mirror_boundary_conditions(_M_x_index, _M_coeff.get_size().x, _M_size2.x);
	mirror_boundary_conditions(_M_y_index, _M_coeff.get_size().y, _M_size2.y);
	
	U result = evaluate();
	bounded<U, T>::apply(result, _M_min, _M_max);
	return round_to<U, T>::value(result); 
}

template <typename T>
T2DVector<T> T2DConvoluteInterpolator<T>::derivative_at(const C2DFVector& x) const
{
	T2DVector<T> result;

	// cut at boundary maybe we can do better
	if (x.x < 0.0 || x.y < 0.0 || x.x >= _M_coeff.get_size().x || x.y >= _M_coeff.get_size().y)
		return result;

	const int xi = _M_kernel->get_indices(x.x, _M_x_index); 
	mirror_boundary_conditions(_M_x_index, _M_coeff.get_size().x, _M_size2.x);
	
	const int yi = _M_kernel->get_indices(x.y, _M_y_index); 
	mirror_boundary_conditions(_M_y_index, _M_coeff.get_size().y, _M_size2.y);

	const double fx = x.x - xi; 
	const double fy = x.y - yi; 
	
	_M_kernel->get_derivative_weights(fx, _M_x_weight); 
	_M_kernel->get_weights(fy, _M_y_weight);


	typename TCoeff2D::value_type r = evaluate();	
	result.x = round_to<typename TCoeff2D::value_type, T>::value(r);  

	
	_M_kernel->get_weights(fx, _M_x_weight); 
	_M_kernel->get_derivative_weights(fy, _M_y_weight); 

	r = evaluate();	
	result.y = round_to<typename TCoeff2D::value_type, T>::value(r);  
	
	return result; 
}

NS_MIA_END



