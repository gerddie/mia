/* -*- mia-c++ -*-
**
** Copyrigh (C) 2004 MPI of Human Cognitive and Brain Sience
**                    Gert Wollny <wollny@cbs.mpg.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  The weight evaluation of the b-splines is based on code by Philippe Thevenaz
  http://bigwww.epfl.ch/

*/

#include <cassert>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#endif


#include <mia/core/interpolator1d.hh>
#include <mia/3d/interpolator.hh>

#include <mia/core/interpolator1d.cxx>
#include <mia/3d/interpolator.cxx>



NS_MIA_BEGIN

using namespace boost;

CInterpolator::~CInterpolator()
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(PSplineKernel kernel, 
					       PSplineBoundaryCondition xbc,  
					       PSplineBoundaryCondition ybc, 
					       PSplineBoundaryCondition zbc):
	m_kernel(kernel), 
	m_xbc(xbc),  
	m_ybc(ybc),  
	m_zbc(zbc)
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(const C3DInterpolatorFactory& o):
	m_kernel(o.m_kernel), 
	m_xbc(o.m_xbc),  
	m_ybc(o.m_ybc),  
	m_zbc(o.m_zbc)
{
}

C3DInterpolatorFactory& C3DInterpolatorFactory::operator = ( const C3DInterpolatorFactory& o)
{
	m_kernel = o.m_kernel;
	m_xbc = o.m_xbc; 
	m_ybc = o.m_ybc; 
	m_zbc = o.m_zbc; 

	return *this;
}

C3DInterpolatorFactory::~C3DInterpolatorFactory()
{
}

PSplineKernel C3DInterpolatorFactory::get_kernel() const
{
	return m_kernel; 
}

EXPORT_3D C3DInterpolatorFactory *create_3dinterpolation_factory(EInterpolation type, EBoundaryConditions bc)
{
	PSplineBoundaryCondition xbc; 
	PSplineBoundaryCondition ybc; 
	PSplineBoundaryCondition zbc; 
	switch (bc) {
	case bc_mirror_on_bounds: 
		xbc = produce_spline_boundary_condition("mirror"); 
		ybc = produce_spline_boundary_condition("mirror"); 
		zbc = produce_spline_boundary_condition("mirror"); 
		break; 
		
	case bc_repeat: 
		xbc = produce_spline_boundary_condition("repeat"); 
		ybc = produce_spline_boundary_condition("repeat"); 
		zbc = produce_spline_boundary_condition("repeat"); 
		break; 
	case bc_zero: 
		xbc = produce_spline_boundary_condition("zero"); 
		ybc = produce_spline_boundary_condition("zero"); 
		zbc = produce_spline_boundary_condition("zero"); 
		break; 
	default: 
		throw invalid_argument("Unknown boundary consitions requested"); 
	}
	
	PSplineKernel kernel; 
	switch (type) {
	case ip_nn: 
	case ip_bspline0: kernel = produce_spline_kernel("bspline:d=0"); break; 
	case ip_linear:
	case ip_bspline1: kernel = produce_spline_kernel("bspline:d=1"); break; 
	case ip_bspline2: kernel = produce_spline_kernel("bspline:d=2"); break; 
	case ip_bspline3: kernel = produce_spline_kernel("bspline:d=3"); break; 
	case ip_bspline4: kernel = produce_spline_kernel("bspline:d=4"); break; 
	case ip_bspline5: kernel = produce_spline_kernel("bspline:d=5"); break; 
	case ip_omoms3:   kernel = produce_spline_kernel("omoms:d=3"); break;
	default: 
		throw invalid_argument("create_interpolator_factory:Unknown interpolator type requested"); 
	}; 

	return new C3DInterpolatorFactory(kernel, xbc, ybc, zbc); 
}


#ifdef __SSE2__
typedef double v2df __attribute__ ((vector_size (16)));
inline void my_daxpy_16_zero(double weight, v2df *in, v2df *out)
{
	v2df w=_mm_set1_pd(weight); 
	
	for (int i = 0; i < 8; ++i) 
		out[i] = in[i] * w;  
}

inline void my_daxpy_16(double weight,  v2df *in,  v2df *out)
{
	v2df w=_mm_set1_pd(weight); 
	
	for (int i = 0; i < 8; ++i)
		out[i] += in[i] * w;

}

inline void my_daxpy_4_zero(double weight, v2df *in, v2df *out)
{
	v2df w=_mm_set1_pd(weight); 
	out[0] = w * in[0]; 
	out[1] = w * in[1]; 
}

inline void my_daxpy_4(double weight, v2df *in, v2df *out)
{
	v2df w=_mm_set1_pd(weight); 
	out[0] += w * in[0]; 
	out[1] += w * in[1]; 
}

/*
  In this function the registration algorithm spends approx 30% of the time 
*/
double add_3d<T3DDatafield< double >, 4>::value(const T3DDatafield< double >&  coeff, 
		    const CSplineKernel::SCache& xc, 
		    const CSplineKernel::SCache& yc,
		    const CSplineKernel::SCache& zc) 
{
	const int dx = coeff.get_size().x; 
	const int dxy = coeff.get_size().x *coeff.get_size().y; 

	v2df  cache[32]; 
	// cache data 
	int idx = 0; 

	// if the boundaries are not mirrored, then we can load without looking at each index 
	// this should happen more often 
	if (xc.is_flat) {
		for (size_t z = 0; z < 4; ++z) {
			const double *slice = &coeff[zc.index[z] * dxy]; 
			for (size_t y = 0; y < 4; ++y, idx+=2) {
				const double *p = &slice[yc.index[y] * dx];
				cache[idx  ] = _mm_loadu_pd(&p[xc.start_idx]);
				cache[idx+1] = _mm_loadu_pd(&p[xc.start_idx+2]);
			}
		}
	}else{
		double __attribute__((aligned(16))) c[4]; 
		for (size_t z = 0; z < 4; ++z) {
			const double *slice = &coeff[zc.index[z] * dxy]; 
			for (size_t y = 0; y < 4; ++y, idx+=2) {
				const double *p = &slice[yc.index[y] * dx];
				c[0] = p[xc.index[0]]; 
				c[1] = p[xc.index[1]]; 
				c[2] = p[xc.index[2]]; 
				c[3] = p[xc.index[3]]; 

				cache[idx  ] = 	_mm_load_pd(&c[0]); 
				cache[idx+1] = 	_mm_load_pd(&c[2]); 
			}
		}
	}
	v2df  target1[8]; 
	// apply splines 
	my_daxpy_16_zero(zc.weights[0], &cache[ 0], target1);
	my_daxpy_16(zc.weights[1], &cache[8], target1);
	my_daxpy_16(zc.weights[2], &cache[16], target1);
	my_daxpy_16(zc.weights[3], &cache[24], target1);

	v2df target2[2];
	my_daxpy_4_zero(yc.weights[0], &target1[ 0], target2);
	my_daxpy_4(yc.weights[1], &target1[ 2], target2);
	my_daxpy_4(yc.weights[2], &target1[ 4], target2);
	my_daxpy_4(yc.weights[3], &target1[6], target2);

	v2df wx[2]; 
	wx[0] = _mm_loadu_pd(&xc.weights[0]); 
	wx[1] = _mm_loadu_pd(&xc.weights[2]);
	
	target2[0] *= wx[0]; 
	target2[1] *= wx[1]; 
	
	target2[0] += target2[1]; 
#ifdef __SSE3__
	double result; 
	target2[0] = _mm_hadd_pd(target2[0], target2[0]); 
	_mm_store_sd(&result, target2[0]); 
	return result; 
#else
	double __attribute__((aligned(16))) r[2]; 
	_mm_store_pd(r, target2[0]); 
	return r[0] + r[1]; 
#endif
}

#endif

#ifdef __SSE__
typedef float v4df __attribute__ ((vector_size (16)));
inline void my_daxpy_16_zero(float weight, v4df *in, v4df *out)
{
	v4df w=_mm_set1_ps(weight); 
	
	for (int i = 0; i < 4; ++i) {
		out[i] = in[i] * w; 
	}
}

inline void my_daxpy_16(float weight, v4df *in, v4df *out)
{
	v4df w=_mm_set1_ps(weight); 
	for (int i = 0; i < 4; ++i) {
		out[i] += in[i] * w; 
	}
}

inline void my_daxpy_4_zero(float weight, v4df* in, v4df *out)
{
	v4df w=_mm_set1_ps(weight); 
	out[0] = w * in[0]; 
}

inline void my_daxpy_4(float weight, v4df* in, v4df *out)
{
	v4df w=_mm_set1_ps(weight); 
	out[0] += w * in[0]; 
}

/*
  In this function the registration algorithm spends approx 30% of the time 
*/
float add_3d<T3DDatafield< float >, 4>::value(const T3DDatafield< float >&  coeff, 
		    const CSplineKernel::SCache& xc, 
		    const CSplineKernel::SCache& yc,
		    const CSplineKernel::SCache& zc) 
{
	const int dx = coeff.get_size().x; 
	const int dxy = coeff.get_size().x *coeff.get_size().y; 
	
	v4df cache[16]; 
	// cache data 
	int idx = 0; 

	// if the boundaries are not mirrored, then we can load without looking at each index 
	// this should happen more often 
	if (xc.is_flat) {
		for (size_t z = 0; z < 4; ++z) {
			const float *slice = &coeff[zc.index[z] * dxy]; 
			for (size_t y = 0; y < 4; ++y, ++idx) {
				const float *p = &slice[yc.index[y] * dx];
				cache[idx] = _mm_loadu_ps(&p[xc.start_idx]);
			}
		}
	}else{
		float __attribute__((aligned(16))) c[4]; 
		for (size_t z = 0; z < 4; ++z) {
			const float *slice = &coeff[zc.index[z] * dxy]; 
			for (size_t y = 0; y < 4; ++y, ++idx) {
				const float *p = &slice[yc.index[y] * dx];
				c[0] = p[xc.index[0]]; 
				c[1] = p[xc.index[1]]; 
				c[2] = p[xc.index[2]]; 
				c[3] = p[xc.index[3]]; 
				cache[idx] = _mm_load_ps(c);
			}
		}
	}
	float wy = yc.weights[0]; 
	v4df  target1[4]; 
	// apply splines 
	my_daxpy_16_zero(zc.weights[0], &cache[ 0], target1);
	my_daxpy_16(zc.weights[1],      &cache[ 4], target1);
	my_daxpy_16(zc.weights[2],      &cache[ 8], target1);
	my_daxpy_16(zc.weights[3],      &cache[12], target1);

	;

	v4df w=_mm_set1_ps(wy); 
	v4df target2 = w * target1[0]; 
	
//	my_daxpy_4_zero(yc.weights[0], &target1[0], &target2);
	my_daxpy_4(yc.weights[1],      &target1[1], &target2);
	my_daxpy_4(yc.weights[2],      &target1[2], &target2);
	my_daxpy_4(yc.weights[3],      &target1[3], &target2);
	
	float __attribute__((aligned(16)))  wxa[4]; 
	copy(xc.weights.begin(), xc.weights.end(), wxa); 
	v4df wx = _mm_load_ps(wxa); 
	target2 *= wx; 
#ifdef __SSE3__	
	float result; 
	target2 = _mm_hadd_ps(target2, target2); 
	target2 = _mm_hadd_ps(target2, target2); 

	_mm_store_ss(&result, target2); 
	return result; 
#else
	float __attribute__((aligned(16))) r[4]; 
	_mm_store_ps(r, target2); 
	return r[0] + r[1] +r[2] + r[3]; 
#endif
}

#endif

#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class T3DInterpolator<TYPE>;		\
	template class T3DConvoluteInterpolator<TYPE>

INSTANCIATE_INTERPOLATORS(bool);
INSTANCIATE_INTERPOLATORS(unsigned char);
INSTANCIATE_INTERPOLATORS(signed char);
INSTANCIATE_INTERPOLATORS(unsigned short);
INSTANCIATE_INTERPOLATORS(signed short);
INSTANCIATE_INTERPOLATORS(unsigned int);
INSTANCIATE_INTERPOLATORS(signed int);
INSTANCIATE_INTERPOLATORS(float);
INSTANCIATE_INTERPOLATORS(double);
#ifdef HAVE_INT64
INSTANCIATE_INTERPOLATORS(mia_int64);
INSTANCIATE_INTERPOLATORS(mia_uint64);
#endif

INSTANCIATE_INTERPOLATORS(C3DFVector);
INSTANCIATE_INTERPOLATORS(C3DDVector);

template class T1DInterpolator<C3DFVector>;
template class T1DConvoluteInterpolator<C3DFVector>;

template class T1DInterpolator<C3DDVector>;
template class T1DConvoluteInterpolator<C3DDVector>;


NS_MIA_END
