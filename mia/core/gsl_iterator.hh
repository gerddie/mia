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

#ifndef gslpp_iterator_hh
#define gslpp_iterator_hh

#include <cassert>
#include <iterator>
#include <iostream>

namespace gsl
{

class vector_iterator
{
public:
       typedef double value_type;
       typedef double *pointer;
       typedef double& reference;
       typedef size_t difference_type;

       vector_iterator(double *base, int stride): m_current(base), m_stride(stride) {}

       vector_iterator(const vector_iterator& other) = default;

       vector_iterator(): m_current(nullptr), m_stride(0) {}

       double& operator *()
       {
              return *m_current;
       };

       double *operator ->()
       {
              return  m_current;
       };

       vector_iterator& operator ++()
       {
              m_current += m_stride;
              return *this;
       };

       vector_iterator operator ++(int)
       {
              vector_iterator result(*this);
              ++(*this);
              return result;
       }


       vector_iterator& operator --()
       {
              m_current -= m_stride;
              return *this;
       };

       vector_iterator operator --(int)
       {
              vector_iterator result(*this);
              --(*this);
              return result;
       }

       vector_iterator& operator += (int dist)
       {
              m_current += dist * m_stride;
              return *this;
       }

       vector_iterator& operator -= (int dist)
       {
              m_current -= dist * m_stride;
              return *this;
       }

       double& operator[] (int idx)
       {
              return m_current[idx * m_stride];
       }

       difference_type operator - (const vector_iterator& other)
       {
              assert(m_stride == other.m_stride);
              return (m_current - other.m_current) / m_stride;
       }

       bool operator == (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current == other.m_current;
       }

       bool operator != (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current != other.m_current;
       }

       bool operator < (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current < other.m_current;
       }

       bool operator <= (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current <= other.m_current;
       }

       bool operator > (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current > other.m_current;
       }

       bool operator >= (const vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current >= other.m_current;
       }

private:
       friend class const_vector_iterator;
       double *m_current;
       int m_stride;
};


inline vector_iterator operator + (const vector_iterator& it, int dist)
{
       vector_iterator result(it);
       result += dist;
       return result;
}

inline vector_iterator operator - (const vector_iterator& it, int dist)
{
       vector_iterator result(it);
       result -= dist;
       return result;
}

inline vector_iterator operator + (int dist, const vector_iterator& it)
{
       vector_iterator result(it);
       result += dist;
       return result;
}


class const_vector_iterator
{
public:
       typedef const double value_type;
       typedef const double *pointer;
       typedef const double& reference;
       typedef size_t difference_type;


       const_vector_iterator(const double *base, int stride):
              m_current(base), m_stride(stride)
       {
       }

       const_vector_iterator(const const_vector_iterator& other) = default;

       const_vector_iterator(const vector_iterator& other):
              m_current(other.m_current),
              m_stride(other.m_stride)
       {
       }

       const_vector_iterator():
              m_current(nullptr),
              m_stride(0)
       {
       }

       const double& operator *() const
       {
              return *m_current;
       };

       const double *operator ->() const
       {
              return  m_current;
       };

       const_vector_iterator& operator ++()
       {
              m_current += m_stride;
              return *this;
       };

       const_vector_iterator operator ++(int)
       {
              const_vector_iterator result(*this);
              ++(*this);
              return result;
       }

       const_vector_iterator& operator --()
       {
              m_current -= m_stride;
              return *this;
       };

       const_vector_iterator operator --(int)
       {
              const_vector_iterator result(*this);
              --(*this);
              return result;
       }

       const_vector_iterator& operator += (int dist)
       {
              m_current += dist * m_stride;
              return *this;
       }

       const_vector_iterator& operator -= (int dist)
       {
              m_current -= dist * m_stride;
              return *this;
       }

       difference_type operator - (const const_vector_iterator& other)
       {
              assert(m_stride == other.m_stride);
              return (m_current - other.m_current) / m_stride;
       }

       const double& operator[] (int idx) const
       {
              return m_current[idx * m_stride];
       }

       bool operator == (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current == other.m_current;
       }

       bool operator != (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current != other.m_current;
       }

       bool operator < (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current < other.m_current;
       }

       bool operator <= (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current <= other.m_current;
       }

       bool operator > (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current > other.m_current;
       }

       bool operator >= (const const_vector_iterator& other) const
       {
              assert(m_stride == other.m_stride);
              return m_current >= other.m_current;
       }


private:
       const double *m_current;
       int m_stride;
};


inline const_vector_iterator operator + (const const_vector_iterator& it, int dist)
{
       const_vector_iterator result(it);
       result += dist;
       return result;
}

inline const_vector_iterator operator - (const const_vector_iterator& it, int dist)
{
       const_vector_iterator result(it);
       result -= dist;
       return result;
}

inline const_vector_iterator operator + (int dist, const const_vector_iterator& it)
{
       const_vector_iterator result(it);
       result += dist;
       return result;
}

}

namespace std
{

template <>
class iterator_traits< gsl::const_vector_iterator >
{
public:
       typedef size_t   difference_type;
       typedef double	 value_type;
       typedef const double *pointer;
       typedef const double&	reference;
       typedef random_access_iterator_tag	iterator_category;
};

template <>
class iterator_traits< gsl::vector_iterator >
{
public:
       typedef size_t   difference_type;
       typedef double	 value_type;
       typedef double *pointer;
       typedef double&	reference;
       typedef random_access_iterator_tag	iterator_category;
};

}

#endif
