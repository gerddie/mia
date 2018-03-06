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

#ifndef mia_core_nccsum_hh
#define mia_core_nccsum_hh

#include <utility>
#include <mia/core/defines.hh>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

NS_MIA_BEGIN

class EXPORT_CORE NCCGradHelper
{
public:
       NCCGradHelper():
              m_sumab_by_a2b2(0.0), m_sumab_by_suma2(0.0),
              m_mean_a(0.0), m_mean_b(0.0)
       {
       }

       NCCGradHelper(double sumab_by_a2b2, double sumab_by_suma2, double mean_a, double mean_b):
              m_sumab_by_a2b2(2.0 * sumab_by_a2b2), m_sumab_by_suma2(sumab_by_suma2),
              m_mean_a(mean_a), m_mean_b(mean_b)
       {
       }

       float get_gradient_scale(double a, double b) const
       {
              return m_sumab_by_a2b2 * ( m_sumab_by_suma2 * ( a  - m_mean_a ) - (b - m_mean_b));
       }


private:
       double m_sumab_by_a2b2;
       double m_sumab_by_suma2;
       double m_mean_a;
       double m_mean_b;
};

#ifdef __SSE2__


class EXPORT_CORE NCCSums
{
       typedef double v2df __attribute__ ((vector_size (16)));
public:
       NCCSums(): m_sumab(0.0), m_n(0.0)
       {
              double zero[2] = {0.0, 0.0};
              m_sum = _mm_loadu_pd(zero);
              m_sum2 = m_sum;
       }

       void add(double a, double b)
       {
              v2df val = {static_cast<double>(a), static_cast<double>(b)};
              v2df sq = val * val;
              m_sum += val;
              m_sum2 +=  sq;
              m_sumab += a * b;
              m_n += 1.0;
       }

       NCCSums& operator += (const NCCSums& other)
       {
              m_sum += other.m_sum;
              m_sum2 +=  other.m_sum2;
              m_sumab += other.m_sumab;
              m_n += other.m_n;
              return *this;
       }

       bool has_samples() const
       {
              return m_n > 0;
       }

       double value() const;

       std::pair<double, NCCGradHelper> get_grad_helper() const;

private:
       v2df m_sum;
       v2df m_sum2;
       double m_sumab;
       double m_n;

};

#else

class EXPORT_CORE NCCSums
{
public:
       NCCSums(): m_suma(0.0), m_sumb(0.0),
              m_suma2(0.0), m_sumb2(0.0),
              m_sumab(0.0), m_n(0.0)
       {
       }

       void add(double a, double b)
       {
              m_suma += a;
              m_sumb += b;
              m_suma2 +=  a * a;
              m_sumb2 +=  b * b;
              m_sumab += a * b;
              m_n += 1.0;
       }

       NCCSums& operator += (const NCCSums& other)
       {
              m_suma += other.m_suma;
              m_sumb += other.m_sumb;
              m_suma2 +=  other.m_suma2;
              m_sumb2 +=  other.m_sumb2;
              m_sumab += other.m_sumab;
              m_n += other.m_n;
              return *this;
       }

       double value() const;

       std::pair<double, NCCGradHelper> get_grad_helper() const;


       bool has_samples() const
       {
              return m_n > 0;
       }
private:
       double m_suma;
       double m_sumb;
       double m_suma2;
       double m_sumb2;
       double m_sumab;
       double m_n;
};


#endif


inline NCCSums operator + (const NCCSums& lhs, const NCCSums& rhs)
{
       NCCSums result(lhs);
       result += rhs;
       return result;
}

NS_MIA_END

#endif
