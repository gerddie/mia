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

using namespace std;

C3DWeightCache::C3DWeightCache(int kernel_size,
                               const CSplineBoundaryCondition& xbc,
                               const CSplineBoundaryCondition& ybc,
                               const CSplineBoundaryCondition& zbc):
       x(kernel_size, xbc, kernel_size < 3),
       y(kernel_size, ybc, true),
       z(kernel_size, zbc, true)
{
}


C3DInterpolatorFactory::C3DInterpolatorFactory(const std::string& kernel, const std::string& bc):
       m_kernel(produce_spline_kernel(kernel)),
       m_xbc(produce_spline_boundary_condition(bc)),
       m_ybc(produce_spline_boundary_condition(bc)),
       m_zbc(produce_spline_boundary_condition(bc))
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(PSplineKernel kernel, const std::string& bc):
       m_kernel(kernel),
       m_xbc(produce_spline_boundary_condition(bc)),
       m_ybc(produce_spline_boundary_condition(bc)),
       m_zbc(produce_spline_boundary_condition(bc))
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(PSplineKernel kernel, const CSplineBoundaryCondition& bc):
       m_kernel(kernel),
       m_xbc(bc.clone()),
       m_ybc(bc.clone()),
       m_zbc(bc.clone())
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(PSplineKernel kernel,
              const CSplineBoundaryCondition& xbc,
              const CSplineBoundaryCondition& ybc,
              const CSplineBoundaryCondition& zbc):
       m_kernel(kernel),
       m_xbc(xbc.clone()),
       m_ybc(ybc.clone()),
       m_zbc(zbc.clone())
{
}

C3DInterpolatorFactory::C3DInterpolatorFactory(const C3DInterpolatorFactory& o):
       m_kernel(o.m_kernel),
       m_xbc(o.m_xbc->clone()),
       m_ybc(o.m_ybc->clone()),
       m_zbc(o.m_zbc->clone())
{
}

C3DInterpolatorFactory& C3DInterpolatorFactory::operator = ( const C3DInterpolatorFactory& o)
{
       m_kernel = o.m_kernel;
       m_xbc.reset(o.m_xbc->clone());
       m_ybc.reset(o.m_ybc->clone());
       m_zbc.reset(o.m_zbc->clone());
       return *this;
}

C3DInterpolatorFactory::~C3DInterpolatorFactory()
{
}

PSplineKernel C3DInterpolatorFactory::get_kernel() const
{
       return m_kernel;
}


#ifdef __SSE2__
typedef double v2df __attribute__ ((vector_size (16)));
inline void my_daxpy_16_zero(double weight, v2df *in, v2df *out)
{
       v2df w = _mm_set1_pd(weight);

       for (int i = 0; i < 8; ++i)
              out[i] = in[i] * w;
}

inline void my_daxpy_16(double weight,  v2df *in,  v2df *out)
{
       v2df w = _mm_set1_pd(weight);

       for (int i = 0; i < 8; ++i)
              out[i] += in[i] * w;
}

inline void my_daxpy_4_zero(double weight, v2df *in, v2df *out)
{
       v2df w = _mm_set1_pd(weight);
       out[0] = w * in[0];
       out[1] = w * in[1];
}

inline void my_daxpy_4(double weight, v2df *in, v2df *out)
{
       v2df w = _mm_set1_pd(weight);
       out[0] += w * in[0];
       out[1] += w * in[1];
}


/*
  In this function the registration algorithm spends approx 7% of the time
  if B-spline 1 is used
*/

double add_3d<T3DDatafield< double >, 2>::value(const T3DDatafield< double >&  coeff,
              const C3DWeightCache& cache)
{
       const int dx = coeff.get_size().x;
       const int dxy = coeff.get_size().x * coeff.get_size().y;
       int idx = 0;
       double __attribute__((aligned(16))) c[8];
       v2df xweights = _mm_loadu_pd(&cache.x.weights[0]);
       v2df yweight0 = _mm_load1_pd(&cache.y.weights[0]);
       v2df yweight1 = _mm_load1_pd(&cache.y.weights[1]);
       v2df zweight0 = _mm_load1_pd(&cache.z.weights[0]);
       v2df zweight1 = _mm_load1_pd(&cache.z.weights[1]);

       if (cache.x.is_flat) {
              for (size_t z = 0; z < 2; ++z) {
                     const double *slice = &coeff[cache.z.index[z] * dxy];

                     for (size_t y = 0; y < 2; ++y, idx += 2 ) {
                            const double *p = &slice[cache.y.index[y] * dx];
                            c[idx    ] = p[cache.x.start_idx];
                            c[idx + 1] = p[cache.x.start_idx + 1];
                     }
              }
       } else {
              for (size_t z = 0; z < 2; ++z) {
                     const double *slice = &coeff[cache.z.index[z] * dxy];

                     for (size_t y = 0; y < 2; ++y, idx += 2) {
                            const double *p = &slice[cache.y.index[y] * dx];
                            c[idx    ] = p[cache.x.index[0]];
                            c[idx + 1] = p[cache.x.index[1]];
                     }
              }
       }

       v2df z0y0 = _mm_load_pd(&c[0]);
       v2df z0y1 = _mm_load_pd(&c[2]);
       v2df z1y0 = _mm_load_pd(&c[4]);
       v2df z1y1 = _mm_load_pd(&c[6]);
       v2df zsum_y0 = z0y0 * zweight0 + z1y0 * zweight1;
       v2df zsum_y1 = z0y1 * zweight0 + z1y1 * zweight1;
       v2df zysum = (zsum_y0 * yweight0 + zsum_y1 * yweight1) * xweights;
#ifdef __SSE3__
       double result;
       zysum = _mm_hadd_pd(zysum, zysum);
       _mm_store_sd(&result, zysum);
       return result;
#else
       double __attribute__((aligned(16))) r[2];
       _mm_store_pd(r, zysum);
       return r[0] + r[1];
#endif
}

/*
  In this function the registration algorithm spends approx 30% of the time
*/
double add_3d<T3DDatafield< double >, 4>::value(const T3DDatafield< double >&  coeff,
              const C3DWeightCache& wicache)
{
       const int dx = coeff.get_size().x;
       const int dxy = coeff.get_size().x * coeff.get_size().y;
       v2df  cache[32];
       // cache data
       int idx = 0;

       // if the boundaries are not mirrored, then we can load without looking at each index
       // this should happen more often
       if (wicache.x.is_flat) {
              for (int z = 0; z < 4; ++z) {
                     const double *slice = &coeff[wicache.z.index[z] * dxy];

                     for (int y = 0; y < 4; ++y, idx += 2) {
                            const double *p = &slice[wicache.y.index[y] * dx];
                            cache[idx  ] = _mm_loadu_pd(&p[wicache.x.start_idx]);
                            cache[idx + 1] = _mm_loadu_pd(&p[wicache.x.start_idx + 2]);
                     }
              }
       } else {
              double __attribute__((aligned(16))) c[4];

              for (int z = 0; z < 4; ++z) {
                     const double *slice = &coeff[wicache.z.index[z] * dxy];

                     for (int y = 0; y < 4; ++y, idx += 2) {
                            const double *p = &slice[wicache.y.index[y] * dx];
                            c[0] = p[wicache.x.index[0]];
                            c[1] = p[wicache.x.index[1]];
                            c[2] = p[wicache.x.index[2]];
                            c[3] = p[wicache.x.index[3]];
                            cache[idx  ] = 	_mm_load_pd(&c[0]);
                            cache[idx + 1] = 	_mm_load_pd(&c[2]);
                     }
              }
       }

       v2df  target1[8];
       // apply splines
       my_daxpy_16_zero(wicache.z.weights[0], &cache[ 0], target1);
       my_daxpy_16(wicache.z.weights[1], &cache[8], target1);
       my_daxpy_16(wicache.z.weights[2], &cache[16], target1);
       my_daxpy_16(wicache.z.weights[3], &cache[24], target1);
       v2df target2[2];
       my_daxpy_4_zero(wicache.y.weights[0], &target1[ 0], target2);
       my_daxpy_4(wicache.y.weights[1], &target1[ 2], target2);
       my_daxpy_4(wicache.y.weights[2], &target1[ 4], target2);
       my_daxpy_4(wicache.y.weights[3], &target1[6], target2);
       v2df wx[2];
       wx[0] = _mm_loadu_pd(&wicache.x.weights[0]);
       wx[1] = _mm_loadu_pd(&wicache.x.weights[2]);
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
       v4df w = _mm_set1_ps(weight);

       for (int i = 0; i < 4; ++i) {
              out[i] = in[i] * w;
       }
}

inline void my_daxpy_16(float weight, v4df *in, v4df *out)
{
       v4df w = _mm_set1_ps(weight);

       for (int i = 0; i < 4; ++i) {
              out[i] += in[i] * w;
       }
}

inline void my_daxpy_4_zero(float weight, v4df *in, v4df *out)
{
       v4df w = _mm_set1_ps(weight);
       out[0] = w * in[0];
}

inline void my_daxpy_4(float weight, v4df *in, v4df *out)
{
       v4df w = _mm_set1_ps(weight);
       out[0] += w * in[0];
}

/*
  In this function the registration algorithm spends approx 7% of the time
  if B-spline 1 is used
*/

float add_3d<T3DDatafield< float >, 2>::value(const T3DDatafield< float >&  coeff, const C3DWeightCache& cache)
{
       const int dx = coeff.get_size().x;
       const int dxy = coeff.get_size().x * coeff.get_size().y;
       int idx = 0;
       assert(!cache.x.is_flat);
       float __attribute__((aligned(16))) c[8];
       float __attribute__((aligned(16))) w[4];
       w[0] = cache.x.weights[0];
       w[1] = cache.x.weights[1];
       w[2] = cache.y.weights[0];
       w[3] = cache.y.weights[1];

       for (int z = 0; z < 2; ++z) {
              const float *slice = &coeff[cache.z.index[z] * dxy];

              for (int y = 0; y < 2; ++y, idx += 2) {
                     const float *p = &slice[cache.y.index[y] * dx];
                     c[idx    ] = p[cache.x.index[0]];
                     c[idx + 1] = p[cache.x.index[1]];
              }
       }

       v4df weights = _mm_load_ps(w);
       v4df wz0 = _mm_set1_ps(cache.z.weights[0]);
       v4df wz1 = _mm_set1_ps(cache.z.weights[1]);
       v4df whx = _mm_shuffle_ps(weights, weights, _MM_SHUFFLE(1, 0, 1, 0));
       v4df why = _mm_shuffle_ps(weights, weights, _MM_SHUFFLE(3, 3, 2, 2));
       v4df z0  = _mm_load_ps(&c[0]);
       v4df z1  = _mm_load_ps(&c[4]);
       v4df z = wz0 * z0 + wz1 * z1;
       v4df wp = whx * why;
       v4df res = wp * z;
#ifdef __SSE3__
       float result;
       res = _mm_hadd_ps(res, res);
       res = _mm_hadd_ps(res, res);
       _mm_store_ss(&result, res);
       return result;
#else
       float __attribute__((aligned(16))) r[4];
       _mm_store_ps(r, res);
       return r[0] + r[1] + r[2] + r[3];
#endif
}


/*
  In this function the registration algorithm spends approx 30% of the time
  if B-spline 3 is used
*/
float add_3d<T3DDatafield< float >, 4>::value(const T3DDatafield< float >&  coeff, const C3DWeightCache& wicache)
{
       const int dx = coeff.get_size().x;
       const int dxy = coeff.get_size().x * coeff.get_size().y;
       v4df cache[16];
       // cache data
       int idx = 0;

       // if the boundaries are not mirrored, then we can load without looking at each index
       // this should happen more often
       if (wicache.x.is_flat) {
              for (int z = 0; z < 4; ++z) {
                     const float *slice = &coeff[wicache.z.index[z] * dxy];

                     for (int y = 0; y < 4; ++y, ++idx) {
                            const float *p = &slice[wicache.y.index[y] * dx];
                            cache[idx] = _mm_loadu_ps(&p[wicache.x.start_idx]);
                     }
              }
       } else {
              float __attribute__((aligned(16))) c[4];

              for (int z = 0; z < 4; ++z) {
                     const float *slice = &coeff[wicache.z.index[z] * dxy];

                     for (int y = 0; y < 4; ++y, ++idx) {
                            const float *p = &slice[wicache.y.index[y] * dx];
                            c[0] = p[wicache.x.index[0]];
                            c[1] = p[wicache.x.index[1]];
                            c[2] = p[wicache.x.index[2]];
                            c[3] = p[wicache.x.index[3]];
                            cache[idx] = _mm_load_ps(c);
                     }
              }
       }

       float wy = wicache.y.weights[0];
       v4df  target1[4];
       // apply splines
       my_daxpy_16_zero(wicache.z.weights[0], &cache[ 0], target1);
       my_daxpy_16(wicache.z.weights[1],      &cache[ 4], target1);
       my_daxpy_16(wicache.z.weights[2],      &cache[ 8], target1);
       my_daxpy_16(wicache.z.weights[3],      &cache[12], target1);
       ;
       v4df w = _mm_set1_ps(wy);
       v4df target2 = w * target1[0];
//	my_daxpy_4_zero(wicache.y.weights[0], &target1[0], &target2);
       my_daxpy_4(wicache.y.weights[1],      &target1[1], &target2);
       my_daxpy_4(wicache.y.weights[2],      &target1[2], &target2);
       my_daxpy_4(wicache.y.weights[3],      &target1[3], &target2);
       float __attribute__((aligned(16)))  wxa[4];
       copy(wicache.x.weights.begin(), wicache.x.weights.end(), wxa);
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
       return r[0] + r[1] + r[2] + r[3];
#endif
}

#endif

#define INSTANCIATE_INTERPOLATORS(TYPE)			\
	template class EXPORT_3D T3DInterpolator<TYPE>;		\
	template class EXPORT_3D T3DConvoluteInterpolator<TYPE>

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

INSTANCIATE_INTERPOLATORS(C3DFVector);
INSTANCIATE_INTERPOLATORS(C3DDVector);

template class EXPORT_3D T1DInterpolator<C3DFVector>;
template class EXPORT_3D T1DConvoluteInterpolator<C3DFVector>;

template class EXPORT_3D T1DInterpolator<C3DDVector>;
template class EXPORT_3D T1DConvoluteInterpolator<C3DDVector>;


NS_MIA_END
