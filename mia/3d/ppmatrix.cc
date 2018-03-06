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

#define VSTREAM_DOMAIN "ppmatrix-3d"

#include <cmath>
#include <mia/3d/ppmatrix.hh>
#include <mia/3d/datafield.cxx>
#include <mia/core/threadedmsg.hh>

#include <mia/core/parallel.hh>
#include <numeric>


#ifdef __SSE2__
#include <emmintrin.h>
#endif

using namespace std;

NS_MIA_BEGIN

struct SMatrixCell {
       double vxx;
       double vxz;

       double vxy;
       double vyy;

       double vyz;
       double vzz;
       SMatrixCell();
};

class C3DPPDivcurlMatrixImpl
{
public:
       C3DPPDivcurlMatrixImpl(const C3DBounds& size, const C3DFVector& range,
                              const CSplineKernel& kernel, double wdiv, double wrot);

       template <typename Field>
       double multiply(const Field& coefficients) const;
       double evaluate(const T3DDatafield<C3DFVector>& coefficients, CDoubleVector& gradient) const;
       double evaluate(const T3DDatafield<C3DDVector>& coefficients, CDoubleVector& gradient) const;
       void reset(const C3DBounds& size, const C3DFVector& range, const CSplineKernel& kernel,
                  double wdiv, double wrot);
       C3DBounds m_size;
private:
       double m_wdiv;
       double m_wrot;
       EInterpolation m_type;
       C3DFVector m_range;
       size_t m_nodes;
       int m_ksize;

       T3DDatafield<SMatrixCell> m_P;
};


SMatrixCell::SMatrixCell()
{
       memset(this, 0, sizeof(*this));
}


C3DPPDivcurlMatrix::C3DPPDivcurlMatrix(const C3DBounds& size, const C3DFVector& range,
                                       const CSplineKernel& kernel, double wdiv, double wrot):
       impl(new C3DPPDivcurlMatrixImpl(size, range, kernel, wdiv, wrot))
{
       TRACE_FUNCTION;
}

C3DPPDivcurlMatrix::~C3DPPDivcurlMatrix()
{
       TRACE_FUNCTION;
       delete impl;
}

double C3DPPDivcurlMatrix::operator * (const T3DDatafield<C3DFVector>& coefficients) const
{
       TRACE_FUNCTION;
       return impl->multiply(coefficients);
}

double C3DPPDivcurlMatrix::operator * (const T3DDatafield<C3DDVector>& coefficients) const
{
       TRACE_FUNCTION;
       return impl->multiply(coefficients);
}
double C3DPPDivcurlMatrix::evaluate(const T3DDatafield<C3DDVector>& coefficients, CDoubleVector& gradient) const
{
       TRACE_FUNCTION;
       return impl->evaluate(coefficients, gradient);
}

double C3DPPDivcurlMatrix::evaluate(const T3DDatafield<C3DFVector>& coefficients, CDoubleVector& gradient) const
{
       TRACE_FUNCTION;
       return impl->evaluate(coefficients, gradient);
}


/**\todo helper class to evaluate values only once, should be re-done and moved to the spline kernel */
class CIntegralCache2
{
public:
       CIntegralCache2(const CSplineKernel& kernel, int deg1, int deg2);
       double get(int delta) const;
private:
       const CSplineKernel& m_kernel;
       int m_shift;
       int m_deg1;
       int m_deg2;
       mutable vector<bool> m_valid;
       mutable vector<double> m_values;
};

CIntegralCache2::CIntegralCache2(const CSplineKernel& kernel, int deg1, int deg2):
       m_kernel(kernel),
       m_shift(kernel.size()),
       m_deg1(deg1),
       m_deg2(deg2),
       m_valid(2 * m_shift + 1),
       m_values(2 * m_shift + 1)

{
       fill(m_valid.begin(), m_valid.end(), false);
}

double CIntegralCache2::get(int delta) const
{
       delta +=  m_shift;

       if (delta < 0 || delta >= (int)m_valid.size())
              return 0;

       double result = 0;

       if (m_valid[delta])
              result = m_values[delta];
       else {
              double result = integrate2(m_kernel, m_shift, delta, m_deg1, m_deg2, 1, 0,  2 * m_shift);
              m_values[delta] = result;
              m_valid[delta] = true;
       }

       return result;
}


C3DPPDivcurlMatrixImpl::C3DPPDivcurlMatrixImpl(const C3DBounds& size, const C3DFVector& range,
              const CSplineKernel& kernel,
              double wdiv, double wrot)
{
       reset(size, range, kernel,  wdiv,  wrot);
}

void C3DPPDivcurlMatrixImpl::reset(const C3DBounds& size, const C3DFVector& range, const CSplineKernel& kernel,
                                   double wdiv, double wrot)
{
       if (  m_size == size && wdiv == m_wdiv && wrot == m_wrot &&
             m_range == range && kernel.get_type()  == m_type)
              return;

       m_size = size;
       m_wdiv = wdiv;
       m_wrot = wrot;
       m_range = range;
       m_type = kernel.get_type();
       m_nodes = size.x * size.y * size.z;
       m_P = T3DDatafield<SMatrixCell>(C3DBounds(2 * kernel.size() + 1,
                                       2 * kernel.size() + 1,
                                       2 * kernel.size() + 1));
       C3DFVector h1((size.x - 1) / range.x,
                     (size.y - 1) / range.y,
                     (size.z - 1) / range.z);
       C3DFVector h2(h1.x * h1.x, h1.y * h1.y, h1.z * h1.z);
       C3DFVector h3(h1.x * h2.x, h1.y * h2.y, h1.z * h2.z);
       C3DFVector h4(h1.x * h3.x, h1.y * h3.y, h1.z * h3.z);
       const double global_scale = 1.0 / (h1.x * h1.y * h1.z);
       cvdebug() << "scale = " << global_scale << "\n";
       cvdebug() << "h1 = " << h1 << "\n";
       cvdebug() << "h2 = " << h2 << "\n";
       cvdebug() << "h3 = " << h3 << "\n";
       cvdebug() << "h4 = " << h4 << "\n";
       m_ksize = kernel.size();
       cvinfo() << "ksize = " << m_ksize  << "\n";
       CIntegralCache2 rc00(kernel, 0, 0);
       CIntegralCache2 rc11(kernel, 1, 1);
       CIntegralCache2 rc22(kernel, 2, 2);
       CIntegralCache2 rc21(kernel, 2, 1);
       CIntegralCache2 rc12(kernel, 1, 2);
       CIntegralCache2 rc01(kernel, 0, 1);
       CIntegralCache2 rc10(kernel, 1, 0);
       const double wsum = wdiv + wrot;
       const double wdelta = 2 * (wdiv - wrot);
       auto p = m_P.begin();

       for (int z = -m_ksize; z <= m_ksize; ++z)  {
              const double r00z =        rc00.get(z);
              const double r01z = h1.z * rc01.get(z);
              const double r11z = h2.z * rc11.get(z);
              const double r12z = h3.z * rc12.get(z);
              const double r22z = h4.z * rc22.get(z);

              for (int y = -m_ksize; y <= m_ksize; ++y)  {
                     const double r00y =        rc00.get(y);
                     const double r01y = h1.y * rc01.get(y);
                     const double r10y = h1.y * rc10.get(y);
                     const double r11y = h2.y * rc11.get(y);
                     const double r12y = h3.y * rc12.get(y);
                     const double r21y = h3.y * rc21.get(y);
                     const double r22y = h4.y * rc22.get(y);

                     for (int x = -m_ksize; x <= m_ksize; ++x, ++p) {
                            const double r00x =        rc00.get(x);
                            const double r01x = h1.x * rc01.get(x);
                            const double r10x = h1.x * rc10.get(x);
                            const double r11x = h2.x * rc11.get(x);
                            const double r21x = h3.x * rc21.get(x);
                            const double r22x = h4.x * rc22.get(x);
                            SMatrixCell cell;
                            const double r200200 = r22x * r00y * r00z;
                            const double r020020 = r00x * r22y * r00z;
                            const double r002002 = r00x * r00y * r22z;
                            const double r110110 = r11x * r11y * r00z;
                            const double r101101 = r11x * r00y * r11z;
                            const double r011011 = r00x * r11y * r11z;
                            cell.vxx = global_scale * (
                                              r002002 * wrot +
                                              r011011 * 2.0 * wrot +
                                              r020020 * wrot +
                                              r101101 * wsum +
                                              r110110 * wsum +
                                              r200200 * wdiv);
                            cell.vyy = global_scale * (
                                              r002002 * wrot +
                                              r011011 * wsum +
                                              r020020 * wdiv +
                                              r101101 * 2 * wrot +
                                              r110110 * wsum +
                                              r200200 * wrot);
                            cell.vzz = global_scale * (
                                              r002002 * wdiv +
                                              r011011 * wsum +
                                              r020020 * wrot +
                                              r101101 * wsum +
                                              r110110 * wrot * 2 +
                                              r200200 * wrot);

                            if (wdelta != 0.0) {
                                   cell.vxy = global_scale * wdelta *
                                              (r21x * r01y * r00z +
                                               r10x * r12y * r00z +
                                               r01x * r10y * r11z);
                                   cell.vxz = global_scale * wdelta *
                                              (r21x * r00y * r01z +
                                               r10x * r11y * r01z +
                                               r10x * r00y * r12z);
                                   cell.vyz = global_scale * wdelta * (
                                                     r11x * r10y * r01z +
                                                     r00x * r21y * r01z +
                                                     r00x * r10y * r12z);
                            }

                            *p = cell;
                     }
              }
       }

       cvdebug() << "P-matrix has " << m_P.size() << " entries\n";
       cvdebug() << "size:" << m_size << ", m_ksize=" << m_ksize << "\n";
}

template <typename Field>
double C3DPPDivcurlMatrixImpl::multiply(const Field& coefficients) const
{
       assert(coefficients.size() == m_nodes);
       double result_1 = 0.0;
       double result_2 = 0.0;
       double result_3 = 0.0;
       double result_4 = 0.0;
       double result_5 = 0.0;
       double result_6 = 0.0;
       const int nx = m_size.x;
       const int ny = m_size.y;
       const int nz = m_size.z;
       auto ci = coefficients.begin();

       if (m_wdiv == m_wrot) {
              for (int zi = 0; zi < nz; ++zi) {
                     for (int yi = 0; yi < ny; ++yi) {
                            for (int xi = 0; xi < nx; ++xi, ++ci) {
                                   for (int zj = max(0, zi - m_ksize); zj < min(zi + m_ksize, nz); ++zj) {
                                          int dz = zi - zj + m_ksize;

                                          for (int yj = max(0, yi - m_ksize); yj < min(yi + m_ksize, ny); ++yj) {
                                                 int dy = yi - yj + m_ksize;
                                                 int xstart = max(0, xi - m_ksize);
                                                 auto cj = coefficients.begin_at(xstart, yj, zj);
                                                 auto p = m_P.begin_at(xi - xstart + m_ksize, dy, dz);

                                                 for (int xj = xstart; xj < min(xi + m_ksize, nx); --p, ++xj, ++cj) {
                                                        result_1 += ci->x * cj->x * p->vxx;
                                                        result_2 += ci->y * cj->y * p->vyy;
                                                        result_3 += ci->z * cj->z * p->vzz;
                                                 }
                                          }
                                   }
                            }
                     }
              }
       } else {
              for (int zi = 0; zi < nz; ++zi) {
                     for (int yi = 0; yi < ny; ++yi) {
                            for (int xi = 0; xi < nx; ++xi, ++ci) {
                                   for (int zj = max(0, zi - m_ksize); zj < min(zi + m_ksize, nz); ++zj) {
                                          const int dz = zi - zj + m_ksize;

                                          for (int yj = max(0, yi - m_ksize); yj < min(yi + m_ksize, ny); ++yj) {
                                                 const int dy = yi - yj + m_ksize;
                                                 const int xstart = max(0, xi - m_ksize);
                                                 const int xend = min(xi + m_ksize, nx);
                                                 auto cj = coefficients.begin_at(xstart, yj, zj);
                                                 auto p = m_P.begin_at(xi - xstart + m_ksize, dy, dz);

                                                 for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {
                                                        result_1 += ci->x * cj->x * p->vxx;
                                                        result_2 += ci->y * cj->y * p->vyy;
                                                        result_3 += ci->z * cj->z * p->vzz;
                                                        result_4 += ci->x * cj->y * p->vxy;
                                                        result_5 += ci->x * cj->z * p->vxz;
                                                        result_6 += ci->y * cj->z * p->vyz;
                                                 }
                                          }
                                   }
                            }
                     }
              }
       }

       return result_1 + result_2 + result_3 +
              result_4 + result_5 + result_6;
}

#ifdef __SSE2__
template <typename Vector>
struct EvaluateGradientAndValue {
       const T3DDatafield<Vector>& coefficients;
       CDoubleVector& gradient;
       vector<double>& values;
       const T3DDatafield<SMatrixCell>& P;
       C3DBounds size;
       int m_ksize;
       EvaluateGradientAndValue(const T3DDatafield<Vector>& _coefficients,
                                CDoubleVector& _gradient,
                                vector<double>& _values,
                                const T3DDatafield<SMatrixCell>& _P,
                                const C3DBounds& _size,
                                int ksize
                               ):
              coefficients(_coefficients),
              gradient(_gradient),
              values(_values),
              P(_P),
              size(_size),
              m_ksize(ksize)
       {
       }

       void operator()( const C1DParallelRange& range ) const
       {
              CThreadMsgStream thread_stream;
              TRACE_FUNCTION;
              int nx = size.x;
              int ny = size.y;
              int nz = size.z;
              double __attribute__((aligned(16))) result[6];

              for (int zi = range.begin(); zi != range.end(); ++zi) {
                     fill(result, result + 6, 0.0);
                     auto ci = coefficients.begin_at(0, 0, zi);
                     auto gi = gradient.begin() + size.x * size.y * 3 * zi;

                     for (int yi = 0; yi < ny; ++yi) {
                            for (int xi = 0; xi < nx; ++xi, ++ci, gi += 3) {
                                   double __attribute__((aligned(16))) g[4];
                                   fill(g, g + 4, 0.0);

                                   for (int zj = max(0, zi - m_ksize); zj < min(zi + m_ksize, nz); ++zj) {
                                          const int dz = zi - zj + m_ksize;

                                          for (int yj = max(0, yi - m_ksize); yj < min(yi + m_ksize, ny); ++yj) {
                                                 const int dy = yi - yj + m_ksize;
                                                 const int xstart = max(0, xi - m_ksize);
                                                 const int xend = min(xi + m_ksize, nx);
                                                 auto p = P.begin_at(xi - xstart + m_ksize, dy, dz);
                                                 auto cj = coefficients.begin_at(xstart, yj, zj);

                                                 for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {
                                                        const __m128d cj_x = _mm_set1_pd(cj->x);
                                                        const __m128d cj_y = _mm_set1_pd(cj->y);
                                                        const __m128d cj_z = _mm_set1_pd(cj->z);
                                                        const __m128d cj_zx = _mm_shuffle_pd(cj_z, cj_x,
                                                                                             _MM_SHUFFLE2(0, 0));
                                                        const __m128d p_xx_xz = _mm_load_pd(&p->vxx);
                                                        const __m128d p_xy_yy = _mm_load_pd(&p->vxy);
                                                        const __m128d p_yz_zz = _mm_load_pd(&p->vyz);
                                                        const __m128d p_xz_xy = _mm_shuffle_pd(p_xx_xz, p_xy_yy,
                                                                                               _MM_SHUFFLE2(0, 1));
                                                        __m128d gz01 = cj_zx * p_xz_xy;
                                                        __m128d gz2X = _mm_mul_sd(cj_y, p_yz_zz);
                                                        const __m128d cj_xp_xx_xz = p_xx_xz * cj_x;
                                                        const __m128d cj_yp_xy_yy = p_xy_yy * cj_y;
                                                        const __m128d cj_zp_yz_zz = p_yz_zz * cj_z;
                                                        const __m128d cj_xyp_xx_xy = _mm_shuffle_pd(cj_xp_xx_xz,
                                                                                     cj_yp_xy_yy,
                                                                                     _MM_SHUFFLE2(0, 0));
                                                        const __m128d cj_yzp_yy_yz = _mm_shuffle_pd(cj_yp_xy_yy,
                                                                                     cj_zp_yz_zz,
                                                                                     _MM_SHUFFLE2(0, 1));
                                                        const __m128d cj_zxp_zz_xz = _mm_shuffle_pd(cj_zp_yz_zz,
                                                                                     cj_xp_xx_xz,
                                                                                     _MM_SHUFFLE2(1, 1));
                                                        const __m128d cix = _mm_set1_pd(ci->x);
                                                        const __m128d ciy = _mm_set1_pd(ci->y);
                                                        const __m128d ciz = _mm_set1_pd(ci->z);
                                                        __m128d r0 = _mm_load_pd(&result[0]);
                                                        __m128d r1 = _mm_load_pd(&result[2]);
                                                        __m128d r2 = _mm_load_pd(&result[4]);
                                                        r0 += cix * cj_xyp_xx_xy;
                                                        r1 += ciy * cj_yzp_yy_yz;
                                                        r2 += ciz * cj_zxp_zz_xz;
                                                        _mm_store_pd(&result[0], r0);
                                                        _mm_store_pd(&result[2], r1);
                                                        _mm_store_pd(&result[4], r2);
                                                        const __m128d cjxy_xx_yy = _mm_shuffle_pd(cj_xp_xx_xz,
                                                                                   cj_yp_xy_yy,
                                                                                   _MM_SHUFFLE2(1, 0));
                                                        const __m128d cjyz_xy_yz = _mm_shuffle_pd(cj_yp_xy_yy,
                                                                                   cj_zp_yz_zz,
                                                                                   _MM_SHUFFLE2(0, 0));
                                                        const __m128d two = _mm_set1_pd(2.0);
                                                        gz01 += two * cjxy_xx_yy + cjyz_xy_yz;
                                                        const __m128d cj_zp_zz = _mm_shuffle_pd(cj_zp_yz_zz,
                                                                                                cj_zp_yz_zz,
                                                                                                _MM_SHUFFLE2(1, 1));
                                                        const __m128d cj_xp_xz = _mm_shuffle_pd(cj_xp_xx_xz,
                                                                                                cj_xp_xx_xz,
                                                                                                _MM_SHUFFLE2(1, 1));
                                                        gz2X += _mm_add_sd(_mm_mul_sd(two, cj_zp_zz), cj_xp_xz);
                                                        __m128d g0 = _mm_load_pd(&g[0]);
                                                        __m128d g2 = _mm_load_pd(&g[2]);
                                                        g0 += gz01;
                                                        g2 += gz2X;
                                                        _mm_store_pd(&g[0], g0);
                                                        _mm_store_pd(&g[2], g2);
                                                 }
                                          }
                                   }

                                   gi[0] = g[0];
                                   gi[1] = g[1];
                                   gi[2] = g[2];
                            }
                     }

                     values[zi] = result[0] + result[1] + result[2] +
                                  result[3] + result[4] + result[5];
              }
       }
};
#else
template <typename Vector>
struct EvaluateGradientAndValue {
       const T3DDatafield<Vector>& coefficients;
       CDoubleVector& gradient;
       vector<double>& values;
       const T3DDatafield<SMatrixCell>& P;
       C3DBounds size;
       int m_ksize;
       EvaluateGradientAndValue(const T3DDatafield<Vector>& _coefficients,
                                CDoubleVector& _gradient,
                                vector<double>& _values,
                                const T3DDatafield<SMatrixCell>& _P,
                                const C3DBounds& _size,
                                int ksize
                               ):
              coefficients(_coefficients),
              gradient(_gradient),
              values(_values),
              P(_P),
              size(_size),
              m_ksize(ksize)
       {
       }

       void operator()( const C1DParallelRange& range ) const
       {
              CThreadMsgStream thread_stream;
              TRACE_FUNCTION;
              int nx = size.x;
              int ny = size.y;
              int nz = size.z;
              double __attribute__((aligned(16))) result[6];

              for (int zi = range.begin(); zi != range.end(); ++zi) {
                     fill(result, result + 6, 0.0);
                     auto ci = coefficients.begin_at(0, 0, zi);
                     auto gi = gradient.begin() + size.x * size.y * 3 * zi;

                     for (int yi = 0; yi < ny; ++yi) {
                            for (int xi = 0; xi < nx; ++xi, ++ci, gi += 3) {
                                   double __attribute__((aligned(16))) g[3];
                                   fill(g, g + 3, 0.0);

                                   for (int zj = max(0, zi - m_ksize); zj < min(zi + m_ksize, nz); ++zj) {
                                          const int dz = zi - zj + m_ksize;

                                          for (int yj = max(0, yi - m_ksize); yj < min(yi + m_ksize, ny); ++yj) {
                                                 const int dy = yi - yj + m_ksize;
                                                 const int xstart = max(0, xi - m_ksize);
                                                 const int xend = min(xi + m_ksize, nx);
                                                 auto p = P.begin_at(xi - xstart + m_ksize, dy, dz);
                                                 auto cj = coefficients.begin_at(xstart, yj, zj);

                                                 for (int xj = xstart; xj < xend; ++xj, ++cj, --p) {
                                                        const double cjxpvxx = cj->x * p->vxx;
                                                        const double cjypvyy = cj->y * p->vyy;
                                                        const double cjzpvzz = cj->z * p->vzz;
                                                        const double cjypvxy = cj->y * p->vxy;
                                                        const double cjxpvxz = cj->x * p->vxz;
                                                        const double cjzpvyz = cj->z * p->vyz;
                                                        result[0] += ci->x * cjxpvxx;
                                                        result[1] += ci->x * cjypvxy;
                                                        result[2] += ci->y * cjypvyy;
                                                        result[3] += ci->z * cjxpvxz;
                                                        result[4] += ci->y * cjzpvyz;
                                                        result[5] += ci->z * cjzpvzz;
                                                        g[0] += 2 * cjxpvxx + cjypvxy + cj->z * p->vxz;
                                                        g[1] += 2 * cjypvyy + cjzpvyz + cj->x * p->vxy;
                                                        g[2] += 2 * cjzpvzz + cjxpvxz + cj->y * p->vyz;
                                                 }
                                          }
                                   }

                                   gi[0] = g[0];
                                   gi[1] = g[1];
                                   gi[2] = g[2];
                            }
                     }

                     values[zi] = result[0] + result[1] + result[2] +
                                  result[3] + result[4] + result[5];
              }
       }
};
#endif


double C3DPPDivcurlMatrixImpl::evaluate(const T3DDatafield<C3DFVector>& coefficients,
                                        CDoubleVector& gradient) const
{
       assert(coefficients.size() == m_nodes);
       assert(gradient.size() == coefficients.size() * 3);
       vector<double> values(m_size.z);
       EvaluateGradientAndValue<C3DFVector> eval(coefficients, gradient, values, m_P, m_size, m_ksize);
       pfor(C1DParallelRange( 0, m_size.z), eval);
       return accumulate(values.begin(), values.end(), 0.0);
}


double C3DPPDivcurlMatrixImpl::evaluate(const T3DDatafield<C3DDVector>& coefficients,
                                        CDoubleVector& gradient) const
{
       assert(coefficients.size() == m_nodes);
       assert(gradient.size() == coefficients.size() * 3);
       vector<double> values(m_size.z);
       EvaluateGradientAndValue<C3DDVector> eval(coefficients, gradient, values, m_P, m_size, m_ksize);
       pfor(C1DParallelRange( 0, m_size.z), eval);
       return accumulate(values.begin(), values.end(), 0.0);
}


const C3DBounds& C3DPPDivcurlMatrix::get_size() const
{
       TRACE_FUNCTION;
       return impl->m_size;
}

void C3DPPDivcurlMatrix::reset(const C3DBounds& size, const C3DFVector& range, const CSplineKernel& kernel,
                               double wdiv, double wrot)
{
       TRACE_FUNCTION;
       impl->reset(size, range, kernel, wdiv, wrot);
}


NS_MIA_END
