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
#include <gsl/gsl_cblas.h>
#include <mia/core/parallel.hh>

#include <mia/core/threadedmsg.hh>
#include <mia/3d/vectorfield.hh>

#include <mia/3d/datafield.cxx>
#include <mia/2d/datafield.cxx>
#include <mia/3d/iterator.cxx>
#include <mia/2d/iterator.cxx>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

NS_MIA_BEGIN


const char *C3DFVectorfield::data_descr = "3dvf";

EXPORT_3D C3DFVectorfield& operator += (C3DFVectorfield& a, const C3DFVectorfield& b)
{
       assert(a.get_size() == b.get_size());
       C3DFVectorfield help(a.get_size());
       std::copy(a.begin(), a.end(), help.begin());
       auto callback = [&a, &b, &help](const C1DParallelRange & range) {
              for (auto z = range.begin(); z != range.end();  ++z)  {
                     C3DFVectorfield::iterator i = a.begin_at(0, 0, z);
                     C3DFVectorfield::const_iterator u = b.begin_at(0, 0, z);

                     for (size_t y = 0; y < a.get_size().y; ++y)  {
                            for (size_t x = 0; x < a.get_size().x; ++x, ++i, ++u)  {
                                   C3DFVector xi = C3DFVector(x, y, z) - *u;
                                   *i = help.get_interpol_val_at(xi) +  *u;
                            }
                     }
              }
       };
       pfor( C1DParallelRange(0, a.get_size().z, 1), callback);
       return a;
}



template <typename T>
T T3DVectorfield<T>::get_interpol_val_at(const C3DFVector& p) const
{
       // Calculate the coordinates and the distances
       size_t  x = (size_t)p.x;
       size_t  y = (size_t)p.y;
       size_t  z = (size_t)p.z;
       float  fx = p.x - x;
       float  fy = p.y - y;
       float  fz = p.z - z;
       float  dx = 1 - fx;
       float  dy = 1 - fy;
       float  dz = 1 - fz;

       if ( x < this->get_size().x - 1 && y  < this->get_size().y - 1 && z < this->get_size().z - 1 ) {
              const T *ptr = &(*this)[x + this->get_size().x * (y + this->get_size().y * z)];
              const T *ptr_h = &ptr[this->get_size().x];
              const T *ptr2 = &ptr[this->get_plane_size_xy()];
              const T *ptr2_h = &ptr2[this->get_size().x];
              const T a1 = T(dx * ptr[0]    + fx * ptr[1]);
              const T a3 = T(dx * ptr_h[0]  + fx * ptr_h[1]);
              const T a5 = T(dx * ptr2[0]   + fx * ptr2[1]);
              const T a7 = T(dx * ptr2_h[0] + fx * ptr2_h[1]);
              const T b1 = T(dy * a1 + fy * a3);
              const T b2 = T(dy * a5 + fy * a7);
              return  T(dz * b1 + fz * b2);
       } else {
              const T a1 = T(dx * (*this)(x, y, z  ) + fx * (*this)(x + 1, y, z  ));
              const T a3 = T(dx * (*this)(x, y + 1, z  ) + fx * (*this)(x + 1, y + 1, z  ));
              const T a5 = T(dx * (*this)(x, y, z + 1) + fx * (*this)(x + 1, y, z + 1));
              const T a7 = T(dx * (*this)(x, y + 1, z + 1) + fx * (*this)(x + 1, y + 1, z + 1));
              const T b1 = T(dy * a1 + fy * a3);
              const T b2 = T(dy * a5 + fy * a7);
              return  T(dz * b1 + fz * b2);
       }
}

void C3DFVectorfield::update_as_inverse_of(const C3DFVectorfield& other, float tol, int maxiter)
{
       assert(get_size() == other.get_size());
       float tol2 = tol * tol;
       C3DLinearVectorfieldInterpolator t(other);
       C1DParallelRange range(0, get_size().z, 1);
       auto callback = [this, &t, tol2, maxiter](const C1DParallelRange & range) {
              CThreadMsgStream msg;

              for (auto z = range.begin(); z != range.end();  ++z)  {
                     auto i = begin_at(0, 0, z);

                     for (size_t y = 0; y < get_size().y; ++y)  {
                            for (size_t x = 0; x < get_size().x; ++x, ++i)  {
                                   int iter = 0;
                                   C3DFVector pos(x, y, z);
                                   float dnorm = 0.0;

                                   while (iter++ < maxiter) {
                                          C3DFVector r = pos - *i;
                                          C3DFVector ov = t(r);
                                          C3DFVector i_delta = r - ov - pos;
                                          dnorm = i_delta.norm2();

                                          if ( dnorm < tol2) {
                                                 break;
                                          }

                                          *i += 0.5 * i_delta;
                                   }
                            }
                     }
              }
       };
       pfor( range, callback );
}

void C3DFVectorfield::update_by_velocity(const C3DFVectorfield& velocity_field, float time_step)
{
       // hoping that the cblas implementation takes care of threading
       cblas_saxpy(velocity_field.size() * 3, time_step, &velocity_field[0].x, 1, &(*this)[0].x, 1);
}

struct C3DLinearVectorfieldInterpolatorImpl {
       C3DLinearVectorfieldInterpolatorImpl(const C3DFVector& x);
#ifdef __SSE__
       __m128 last_elm;
#endif
       size_t m_field_size_m2;
};

C3DLinearVectorfieldInterpolatorImpl::C3DLinearVectorfieldInterpolatorImpl(const C3DFVector& v)
{
#ifdef __SSE__
       last_elm = _mm_set_ps(0.0f, v.z, v.y, v.x);
#endif
}

C3DLinearVectorfieldInterpolator::C3DLinearVectorfieldInterpolator(const C3DFVectorfield& field):
       m_field(field),
       m_save_index_range(m_field.size() - m_field.get_plane_size_xy()
                          - m_field.get_size().x - 2),
       m_field_size_m1(m_field.size() - 1),
       impl(nullptr)
{
#ifdef __SSE__
       impl = new C3DLinearVectorfieldInterpolatorImpl(m_field[m_field_size_m1]);
#endif
       impl->m_field_size_m2 = m_field.size() - 2;
}

C3DLinearVectorfieldInterpolator::~C3DLinearVectorfieldInterpolator()
{
#ifdef __SSE__
       delete impl;
#endif
}

#ifdef __SSE__
C3DFVector C3DLinearVectorfieldInterpolator::operator()(const C3DFVector& x) const
{
       C3DFVector result = C3DFVector::_0;
       const C3DFVector h(floor(x.x), floor(x.y), floor(x.z));
       const C3DBounds ip(static_cast<unsigned int>(h.x),
                          static_cast<unsigned int>(h.y),
                          static_cast<unsigned int>(h.z));

       if (ip < m_field.get_size()) {
              size_t linear_index = ip.x + m_field.get_size().x * (ip.y  + ip.z * m_field.get_size().y);
              const C3DFVector w1 = x - h;

              if (w1 == C3DFVector::_0) {
                     // early exist for this special case
                     result = m_field[linear_index];
              } else {
                     // note that the 4th value of the SSE register contains garbage,
                     // i.e. loads the x value of the next vector in the field.
                     //
                     const C3DFVector w0 = C3DFVector::_1 - w1;
                     __m128 in[8];

                     if (linear_index < m_save_index_range) {
                            // in this case all the values we access are certain to be
                            // within the allocated range
                            in[0] = _mm_loadu_ps(&m_field[linear_index].x);
                            in[1] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                            const size_t linear_index_y1 = linear_index + m_field.get_size().x;
                            in[2] = _mm_loadu_ps(&m_field[linear_index_y1].x);
                            in[3] = _mm_loadu_ps(&m_field[linear_index_y1 + 1].x);
                            linear_index +=  m_field.get_plane_size_xy();
                            in[4] = _mm_loadu_ps(&m_field[linear_index].x);
                            in[5] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                            linear_index +=  m_field.get_size().x;
                            in[6] = _mm_loadu_ps(&m_field[linear_index].x);
                            in[7] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                     } else {
                            // We can not assume that all access would be to the data inside
                            // the allocated range
                            memset(in, 0, 8 * sizeof(__m128));

                            if (linear_index < m_field_size_m1)
                                   in[0] = _mm_loadu_ps(&m_field[linear_index].x);
                            else
                                   in[0] = impl->last_elm;

                            if (linear_index < impl->m_field_size_m2)
                                   in[1] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                            else
                                   in[1] = impl->last_elm;

                            size_t linear_index_y1 = linear_index + m_field.get_size().x;

                            if (linear_index_y1 < impl->m_field_size_m2) {
                                   in[2] = _mm_loadu_ps(&m_field[linear_index_y1].x);
                                   in[3] = _mm_loadu_ps(&m_field[linear_index_y1 + 1].x);
                            } else if (linear_index_y1 == impl->m_field_size_m2) {
                                   in[2] = _mm_loadu_ps(&m_field[linear_index_y1].x);
                                   in[3] = impl->last_elm;
                            } else if (linear_index_y1 == m_field_size_m1) {
                                   in[2] = impl->last_elm;
                            }

                            linear_index +=  m_field.get_plane_size_xy();

                            if (linear_index < impl->m_field_size_m2) {
                                   in[4] = _mm_loadu_ps(&m_field[linear_index].x);
                                   in[5] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                            } else if (linear_index == impl->m_field_size_m2) {
                                   in[4] = _mm_loadu_ps(&m_field[linear_index].x);
                                   in[5] = impl->last_elm;
                            } else if (linear_index == m_field_size_m1) {
                                   in[4] = impl->last_elm;
                            }

                            linear_index +=  m_field.get_size().x;

                            if (linear_index < impl->m_field_size_m2) {
                                   in[6] = _mm_loadu_ps(&m_field[linear_index].x);
                                   in[7] = _mm_loadu_ps(&m_field[linear_index + 1].x);
                            } else if (linear_index == impl->m_field_size_m2) {
                                   in[6] = _mm_loadu_ps(&m_field[linear_index].x);
                                   in[7] = impl->last_elm;
                            } else if (linear_index == m_field_size_m1) {
                                   in[6] = impl->last_elm;
                            }
                     }

                     const __m128 w0x = _mm_set1_ps(w0.x);
                     const __m128 w1x = _mm_set1_ps(w1.x);
                     const __m128 w0y = _mm_set1_ps(w0.y);
                     const __m128 w1y = _mm_set1_ps(w1.y);
                     const __m128 w0z = _mm_set1_ps(w0.z);
                     const __m128 w1z = _mm_set1_ps(w1.z);
                     const __m128 r =
                            w0z * (w0y * ( w0x * in[0] + w1x * in[1]) +
                                   w1y * ( w0x * in[2] + w1x * in[3])) +
                            w1z * (w0y * ( w0x * in[4] + w1x * in[5]) +
                                   w1y * ( w0x * in[6] + w1x * in[7]));
                     float __attribute__((aligned(16))) result_help[4];
                     _mm_store_ps(result_help, r);
                     result.x = result_help[0];
                     result.y = result_help[1];
                     result.z = result_help[2];
              }
       }

       return result;
}
#else // no sse 
C3DFVector C3DLinearVectorfieldInterpolator::operator ()(const C3DFVector& x) const
{
       C3DFVector result = C3DFVector::_0;
       const C3DFVector h(floor(x.x), floor(x.y), floor(x.z));
       const C3DBounds ip(static_cast<unsigned int>(h.x),
                          static_cast<unsigned int>(h.y),
                          static_cast<unsigned int>(h.z));

       if (ip < m_field.get_size()) {
              size_t linear_index = ip.x + m_field.get_size().x * (ip.y  + ip.z * m_field.get_size().y);
              const C3DFVector& in000 = m_field[linear_index];
              const C3DFVector w1 = x - h;

              if (w1 == C3DFVector::_0) {
                     // early exist for this special case
                     result = in000;
              } else {
                     const C3DFVector w0 = C3DFVector::_1 - w1;

                     if (linear_index < m_save_index_range) {
                            // with above test we don't need  check for boundaries
                            const C3DFVector& in001 = m_field[linear_index + 1];
                            size_t linear_index_y1 = linear_index + m_field.get_size().x;
                            const C3DFVector& in010 = m_field[linear_index_y1];
                            const C3DFVector& in011 = m_field[linear_index_y1 + 1];
                            linear_index +=  m_field.get_plane_size_xy();
                            const C3DFVector& in100 = m_field[linear_index];
                            const C3DFVector& in101 = m_field[linear_index + 1];
                            linear_index +=  m_field.get_size().x;
                            const C3DFVector& in110 = m_field[linear_index];
                            // here we have to check, because otherwise we might access
                            // data outside the allocated memory.
                            ++linear_index;
                            const C3DFVector& in111 = m_field[linear_index];
                            // now interpolate
                            result = w0.z * (w0.y * ( w0.x * in000 + w1.x * in001) +
                                             w1.y * ( w0.x * in010 + w1.x * in011)) +
                                     w1.z * (w0.y * ( w0.x * in100 + w1.x * in101) +
                                             w1.y * ( w0.x * in110 + w1.x * in111));
                     } else  {
                            // use the slow operator that always tests to get the
                            // elements
                            const C3DFVector& in001 = m_field(ip.x + 1, ip.y, ip.z);
                            const C3DFVector& in010 = m_field(ip.x, ip.y + 1, ip.z);
                            const C3DFVector& in011 = m_field(ip.x + 1, ip.y + 1, ip.z);
                            const C3DFVector& in100 = m_field(ip.x, ip.y, ip.z + 1);
                            const C3DFVector& in101 = m_field(ip.x + 1, ip.y, ip.z + 1);
                            const C3DFVector& in110 = m_field(ip.x, ip.y + 1, ip.z + 1);
                            const C3DFVector& in111 = m_field(ip.x + 1, ip.y + 1, ip.z + 1);
                            result = w0.z * (w0.y * ( w0.x * in000 + w1.x * in001) +
                                             w1.y * ( w0.x * in010 + w1.x * in011)) +
                                     w1.z * (w0.y * ( w0.x * in100 + w1.x * in101) +
                                             w1.y * ( w0.x * in110 + w1.x * in111)); ;
                     }
              }
       }

       return result;
}
#endif

#define INSTANCIATE(TYPE)						\
	template class  T3DDatafield<TYPE>;				\
	template class  range3d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator<T3DDatafield<TYPE>::const_iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::const_iterator>;


#define INSTANCIATE2D(TYPE)						\
	template class  EXPORT_3D T2DDatafield<TYPE>;			\
	template class  range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class  range2d_iterator<T2DDatafield<TYPE>::const_iterator>;


#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif

INSTANCIATE2D(C3DFVector);
INSTANCIATE2D(C3DDVector);

INSTANCIATE(C3DFVector);
INSTANCIATE(C3DDVector);
template class EXPORT_3D T3DVectorfield<C3DFVector>;
template class EXPORT_3D T3DVectorfield<C3DDVector>;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif




NS_MIA_END
