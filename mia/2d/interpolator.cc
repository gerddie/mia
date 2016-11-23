/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <cassert>
#include <cstring>

#ifdef __SSE3__
#include <pmmintrin.h>
#endif


#include <mia/core/interpolator1d.hh>
#include <mia/2d/interpolator.hh>

#include <mia/core/interpolator1d.cxx>
#include <mia/2d/interpolator.cxx>

NS_MIA_BEGIN

using namespace std; 

C2DInterpolatorFactory::C2DInterpolatorFactory(const std::string& kernel, const std::string& bc):
	m_kernel(produce_spline_kernel(kernel)), 
	m_xbc(produce_spline_boundary_condition(bc)),
	m_ybc(produce_spline_boundary_condition(bc))
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& bc):
	m_kernel(kernel), 
	m_xbc(bc.clone()),
	m_ybc(bc.clone())
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(PSplineKernel kernel, 
					       const CSplineBoundaryCondition& xbc, const CSplineBoundaryCondition& ybc):
	m_kernel(kernel), 
	m_xbc(xbc.clone()),
	m_ybc(ybc.clone())
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(PSplineKernel kernel, const std::string& bc):
	m_kernel(kernel), 
	m_xbc(produce_spline_boundary_condition(bc)),
	m_ybc(produce_spline_boundary_condition(bc))
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(const C2DInterpolatorFactory& o):
	m_kernel(o.m_kernel), 
	m_xbc(o.m_xbc->clone()),
	m_ybc(o.m_ybc->clone())

{
}

C2DInterpolatorFactory& C2DInterpolatorFactory::operator = ( const C2DInterpolatorFactory& o)
{
	m_kernel = o.m_kernel;
	m_xbc.reset(o.m_xbc->clone()); 
	m_ybc.reset(o.m_ybc->clone()); 
		    
	return *this;
}

C2DInterpolatorFactory::~C2DInterpolatorFactory()
{
}

const CSplineKernel* C2DInterpolatorFactory::get_kernel() const
{
	return m_kernel.get();
}

#ifdef __SSE__
float add_2d_new<T2DDatafield< float >, 4>::value(const T2DDatafield< float >&  coeff, 
							   const CSplineKernel::SCache& xc, 
							   const CSplineKernel::SCache& yc) 
{
	typedef float v4df __attribute__ ((vector_size (16)));
	float __attribute__((aligned(16))) cache[16]; 
	float __attribute__((aligned(16))) xweight[4]; 
	copy(xc.weights.begin(), xc.weights.end(), xweight); 
	
	const int dx = coeff.get_size().x; 
	int idx = 0; 
	if (xc.is_flat) {
		for (size_t y = 0; y < 4; ++y, idx+=4) {
			const float *p = &coeff[yc.index[y] * dx];
			memcpy(&cache[idx], &p[xc.start_idx], 4*sizeof(float));
		}
	}else{
		for (size_t y = 0; y < 4; ++y, idx+=4) {
			const float *p = &coeff[yc.index[y] * dx];
			cache[idx  ] = p[xc.index[0]]; 
			cache[idx+1] = p[xc.index[1]]; 
			cache[idx+2] = p[xc.index[2]]; 
			cache[idx+3] = p[xc.index[3]]; 
		}
	}

	v4df wx  = _mm_loadu_ps(xweight);
	v4df w0  = _mm_set1_ps(yc.weights[0]);
	v4df w1  = _mm_set1_ps(yc.weights[1]);
	v4df w2  = _mm_set1_ps(yc.weights[2]);
	v4df w3  = _mm_set1_ps(yc.weights[3]);

	v4df x0 = _mm_load_ps(&cache[0]); 
	v4df x1 = _mm_load_ps(&cache[4]); 
	v4df x2 = _mm_load_ps(&cache[8]); 
	v4df x3 = _mm_load_ps(&cache[12]); 
	
	v4df res = (x0 * w0 + x1 * w1 + x2 * w2 + x3 * w3 )* wx; 

#ifdef __SSE3__	
	float result; 
	res = _mm_hadd_ps(res, res); 
	res = _mm_hadd_ps(res, res); 

	_mm_store_ss(&result, res); 
	return result; 
#else
	float __attribute__((aligned(16))) r[4]; 
	_mm_store_ps(r, res); 
	return r[0] + r[1] +r[2] + r[3]; 
#endif
}
#endif


#ifdef __SSE2__
double add_2d_new<T2DDatafield< double >, 4>::value(const T2DDatafield< double >&  coeff, 
							   const CSplineKernel::SCache& xc, 
							   const CSplineKernel::SCache& yc) 
{
	typedef double v2df __attribute__ ((vector_size (16)));
	double __attribute__((aligned(16))) cache[16]; 
	
	const int dx = coeff.get_size().x; 
	int idx = 0; 
	if (xc.is_flat) {
		for (size_t y = 0; y < 4; ++y, idx+=4) {
			const double *p = &coeff[yc.index[y] * dx];
			v2df y1 = _mm_loadu_pd(&p[xc.start_idx]);
			v2df y2 = _mm_loadu_pd(&p[xc.start_idx+2]);
			_mm_store_pd(&cache[idx  ], y1); 
			_mm_store_pd(&cache[idx+2], y2); 
		}
	}else{
		for (size_t y = 0; y < 4; ++y, idx+=4) {
			const double *p = &coeff[yc.index[y] * dx];
			cache[idx  ] = p[xc.index[0]]; 
			cache[idx+1] = p[xc.index[1]]; 
			cache[idx+2] = p[xc.index[2]]; 
			cache[idx+3] = p[xc.index[3]]; 
		}
	}
	
	v2df w0  = _mm_set1_pd(yc.weights[0]);
	v2df w1  = _mm_set1_pd(yc.weights[1]);
	
	v2df x00 = _mm_load_pd(&cache[0]); 
	v2df x01 = _mm_load_pd(&cache[2]);
	v2df x10 = _mm_load_pd(&cache[4]); 
	v2df x11 = _mm_load_pd(&cache[6]);
	
	v2df y1 = x00 * w0 + x10 * w1; 
	v2df y2 = x01 * w0 + x11 * w1; 

	v2df w2  = _mm_set1_pd(yc.weights[2]);
	v2df w3  = _mm_set1_pd(yc.weights[3]);
	v2df x20 = _mm_load_pd(&cache[8]); 
	v2df x21 = _mm_load_pd(&cache[10]);
	v2df x30 = _mm_load_pd(&cache[12]); 
	v2df x31 = _mm_load_pd(&cache[14]);

	y1 += x20 * w2 + x30 * w3; 
	y2 += x21 * w2 + x31 * w3; 

	
	w0 = _mm_loadu_pd(&xc.weights[0]); 
	w1 = _mm_loadu_pd(&xc.weights[2]); 
	
	y1 *= w0; 
	y2 *= w1; 
	
	y1 += y2; 
	
	double __attribute__((aligned(16))) result[2]; 
	_mm_storeu_pd(result, y1); 
	
	return result[0] + result[1]; 
}
#endif

#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class EXPORT_2D T2DInterpolator<TYPE>;		\
	template class EXPORT_2D T2DConvoluteInterpolator<TYPE>

INSTANCIATE_INTERPOLATORS(bool);
INSTANCIATE_INTERPOLATORS(int8_t);
INSTANCIATE_INTERPOLATORS(int16_t);
INSTANCIATE_INTERPOLATORS(int32_t);
INSTANCIATE_INTERPOLATORS(int64_t);
INSTANCIATE_INTERPOLATORS(uint8_t);
INSTANCIATE_INTERPOLATORS(uint16_t);
INSTANCIATE_INTERPOLATORS(uint32_t);
INSTANCIATE_INTERPOLATORS(uint64_t);
INSTANCIATE_INTERPOLATORS(float);
INSTANCIATE_INTERPOLATORS(double);


INSTANCIATE_INTERPOLATORS(C2DFVector);

template class EXPORT_2D T1DInterpolator<C2DFVector>;
template class EXPORT_2D T1DConvoluteInterpolator<C2DFVector>;



NS_MIA_END
