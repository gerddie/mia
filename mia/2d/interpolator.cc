/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
 */

/*
  The weight evaluation of the b-splines is based on code by Philippe Thevenaz
  http://bigwww.epfl.ch/

*/

#include <cassert>

#include <mia/core/interpolator1d.hh>
#include <mia/2d/interpolator.hh>

#include <mia/core/interpolator1d.cxx>
#include <mia/2d/interpolator.cxx>

NS_MIA_BEGIN

using namespace boost;

C2DInterpolator::~C2DInterpolator()
{
}


C2DInterpolatorFactory::C2DInterpolatorFactory(EInterpolationFactory type, PBSplineKernel kernel):
	m_type(type),
	m_kernel(kernel)
{
}

C2DInterpolatorFactory::C2DInterpolatorFactory(const C2DInterpolatorFactory& o):
	m_type(o.m_type),
	m_kernel(o.m_kernel)
{
}

C2DInterpolatorFactory& C2DInterpolatorFactory::operator = ( const C2DInterpolatorFactory& o)
{
	m_type = o.m_type;
	m_kernel = o.m_kernel;

	return *this;
}

C2DInterpolatorFactory::~C2DInterpolatorFactory()
{
}

const CBSplineKernel* C2DInterpolatorFactory::get_kernel() const
{
	return m_kernel.get();
}

C2DInterpolatorFactory *create_2dinterpolation_factory(EInterpolation type)
{
	return create_interpolator_factory<C2DInterpolatorFactory>(type); 
}


#ifdef __SSE2__
double add_2d_new<T2DDatafield< double >, 4>::value(const T2DDatafield< double >&  coeff, 
							   const CBSplineKernel::SCache& xc, 
							   const CBSplineKernel::SCache& yc) 
{
	typedef double v2df __attribute__ ((vector_size (16)));
	double __attribute__((aligned(16))) cache[16]; 
	
	const int dx = coeff.get_size().x; 
	int idx = 0; 
	for (size_t y = 0; y < 4; ++y, idx+=4) {
		const double *p = &coeff[yc.index[y] * dx];
		cache[idx  ] = p[xc.index[0]]; 
		cache[idx+1] = p[xc.index[1]]; 
		cache[idx+2] = p[xc.index[2]]; 
		cache[idx+3] = p[xc.index[3]]; 
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
	template class T2DInterpolator<TYPE>;		\
	template class T2DConvoluteInterpolator<TYPE>

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


INSTANCIATE_INTERPOLATORS(C2DFVector);

template class T1DInterpolator<C2DFVector>;
template class T1DConvoluteInterpolator<C2DFVector>;



NS_MIA_END
