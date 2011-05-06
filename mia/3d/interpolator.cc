/*
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


#include <mia/core/interpolator1d.hh>
#include <mia/3d/interpolator.hh>

#include <mia/core/interpolator1d.cxx>
#include <mia/3d/interpolator.cxx>



NS_MIA_BEGIN

using namespace boost;

CInterpolator::~CInterpolator()
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(EInterpolationFactory type, PBSplineKernel kernel):
	m_type(type),
	m_kernel(kernel)
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(const C3DInterpolatorFactory& o):
	m_type(o.m_type),
	m_kernel(o.m_kernel)
{
}

C3DInterpolatorFactory& C3DInterpolatorFactory::operator = ( const C3DInterpolatorFactory& o)
{
	m_type = o.m_type;
	m_kernel = o.m_kernel;

	return *this;
}

C3DInterpolatorFactory::~C3DInterpolatorFactory()
{
}

PBSplineKernel C3DInterpolatorFactory::get_kernel() const
{
	return m_kernel; 
}

EXPORT_3D C3DInterpolatorFactory *create_3dinterpolation_factory(EInterpolation type)
{
	return create_interpolator_factory<C3DInterpolatorFactory>(type); 
}

#ifdef __SSE2__
typedef double v2df __attribute__ ((vector_size (16)));
inline void my_daxpy_16_zero(double weight, double *in, double *out)
{
	v2df w=_mm_set1_pd(weight); 
	
	for (int i = 0; i < 2; ++i, in += 8, out += 8) {
		
		v2df x1 = _mm_load_pd(&in[0]); 
		v2df x2 = _mm_load_pd(&in[2]); 
		v2df x3 = _mm_load_pd(&in[4]); 
		v2df x4 = _mm_load_pd(&in[6]); 
		x1 *= w; 
		x2 *= w; 
		x3 *= w; 
		x4 *= w; 
		_mm_store_pd(&out[0], x1); 
		_mm_store_pd(&out[2], x2); 
		_mm_store_pd(&out[4], x3); 
		_mm_store_pd(&out[6], x4); 
	}
}

inline void my_daxpy_16(double weight, double *in, double *out)
{
	v2df w=_mm_set1_pd(weight); 
	
	for (int i = 0; i < 2; ++i, in += 8, out += 8) {
		
		v2df x1 = _mm_load_pd(&in[0]); 
		v2df x2 = _mm_load_pd(&in[2]); 
		v2df x3 = _mm_load_pd(&in[4]); 
		v2df x4 = _mm_load_pd(&in[6]); 
		v2df y1 = _mm_load_pd(&out[0]); 
		v2df y2 = _mm_load_pd(&out[2]); 
		v2df y3 = _mm_load_pd(&out[4]); 
		v2df y4 = _mm_load_pd(&out[6]); 
		x1 *= w; 
		x2 *= w; 
		x3 *= w; 
		x4 *= w; 
		y1 += x1; 
		y2 += x2; 
		y3 += x3; 
		y4 += x4; 
		_mm_store_pd(&out[0], y1); 
		_mm_store_pd(&out[2], y2); 
		_mm_store_pd(&out[4], y3); 
		_mm_store_pd(&out[6], y4); 
	}
}

inline void my_daxpy_4_zero(double weight, double *in, double *out)
{

	v2df w=_mm_set1_pd(weight); 
	
	v2df x1 = _mm_load_pd(&in[0]); 
	v2df x2 = _mm_load_pd(&in[2]); 
	
	x1 *= w; 
	x2 *= w; 
	_mm_store_pd(&out[0], x1); 
	_mm_store_pd(&out[2], x2); 
}

inline void my_daxpy_4(double weight, double *in, double *out)
{

	v2df w=_mm_set1_pd(weight); 
	
	v2df x1 = _mm_load_pd(&in[0]); 
	v2df x2 = _mm_load_pd(&in[2]); 
	v2df y1 = _mm_load_pd(&out[0]); 
	v2df y2 = _mm_load_pd(&out[2]); 
	
	x1 *= w; 
	x2 *= w; 
	y1 += x1; 
	y2 += x2; 
	_mm_store_pd(&out[0], y1); 
	_mm_store_pd(&out[2], y2); 
}

double add_3d<T3DDatafield< double >, 4>::value(const T3DDatafield< double >&  coeff, 
		    const CBSplineKernel::SCache& xc, 
		    const CBSplineKernel::SCache& yc,
		    const CBSplineKernel::SCache& zc) 
{
	const int dx = coeff.get_size().x; 
	const int dxy = coeff.get_size().x *coeff.get_size().y; 
	
	double  __attribute__((aligned(16))) cache[64]; 
	// cache data 
	int idx = 0; 
	for (size_t z = 0; z < 4; ++z) {
		for (size_t y = 0; y < 4; ++y, idx+=4) {
			const double *p = &coeff[yc.index[y] * dx + zc.index[z] * dxy];

			cache[idx  ] = p[xc.index[0]]; 
			cache[idx+1] = p[xc.index[1]]; 
			cache[idx+2] = p[xc.index[2]]; 
			cache[idx+3] = p[xc.index[3]]; 
			
		}
	}
	double __attribute__((aligned(16))) target1[16]; 
	memset(target1, 0, 16 * sizeof(double)); 
	// apply splines 
	my_daxpy_16_zero(zc.weights[0], &cache[ 0], target1);
	my_daxpy_16(zc.weights[1], &cache[16], target1);
	my_daxpy_16(zc.weights[2], &cache[32], target1);
	my_daxpy_16(zc.weights[3], &cache[48], target1);

	double __attribute__((aligned(16))) target2[4];

	my_daxpy_4_zero(yc.weights[0], &target1[ 0], target2);
	my_daxpy_4(yc.weights[1], &target1[ 4], target2);
	my_daxpy_4(yc.weights[2], &target1[ 8], target2);
	my_daxpy_4(yc.weights[3], &target1[12], target2);
	
	return target2[0] * xc.weights[0] + target2[1] * xc.weights[1] + 
		target2[2] * xc.weights[2] + target2[3] * xc.weights[3]; 
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
