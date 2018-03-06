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

#ifndef mia_core_vector_hh
#define mia_core_vector_hh

#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>
#include <memory>
#include <cstring>
#include <cassert>
#include <ostream>

NS_MIA_BEGIN


/**
   \cond INTERNAL
   \ingroup misc

   Helper structure used to initialize std::shared_ptr with the proper
   delete operator.
 */
template <typename T>
struct array_destructor {
       /// run delete on an array
       virtual void operator () (T *p)
       {
              delete[] p;
       }
};

/**
   \ingroup misc

   Helper structure used to initialize std::shared_ptr with an empty
   delete operator, i.e. to fake a shared pointer
 */
template <typename T>
struct array_void_destructor {
       /// skip deleting the pointer
       virtual void operator () (T *)
       {
       }
};
/// \endcond

/**
   \ingroup misc
   \brief A wrapper around the c-array to provide an STL like interface for iterators

   c-array envelope that supports some facilities of STL like vectors and that
   allows holding pre-allocated data.
   Handling of the optinal deleting of the array is done by a destructor template
   passed to the std::shared_ptr constructor
   \tparam the data type of the C-array
*/

template <typename T>
class TCArrayWrapper
{
public:

       /// \cond STLCOMPAT
       typedef T& reference;
       typedef const T& const_reference;
       typedef T *iterator;
       typedef const T *const_iterator;
       typedef size_t size_type;
       typedef T value_type;
       /// \endcond

       /**
          Create a vector, the data is owned by this vector and will be
          deleted if the reference count reaches zero
          \param n
          \param clean initialize vector to 0
        */
       TCArrayWrapper(size_t n, bool clean = true):
              m_size(n),
              m_data(new T[n], array_destructor<T>()),
              m_cdata(m_data.get())
       {
              if (clean)
                     memset(m_data.get(), 0, m_size * sizeof(T));
       }

       /** copy constructor, this is a shallow copy, i.e. the data is shared
           between the original and the copied vector
           \param other
       */
       TCArrayWrapper(const TCArrayWrapper<T>& other):
              m_size(other.m_size),
              m_data(other.m_data),
              m_cdata(other.m_cdata)
       {
       }

       /// assignment operator
       TCArrayWrapper<T>& operator = (const TCArrayWrapper<T>& other)
       {
              m_size = other.m_size;
              m_data = other.m_data;
              m_cdata = other.m_cdata;
              return *this;
       }

       /**
          Constructor that creates the STL-like vector as an envelop around a
          C-array. The data will not be freed at destruction time.
          \param n size of input array
          \param init allocated input data
        */
       TCArrayWrapper(size_t n, T *init):
              m_size(n),
              m_data(init, array_void_destructor<T>()),
              m_cdata(init)
       {
       }

       /**
          Constructor that creates the STL-like vector as an envelop around a
          C-array. The data will not be freed at destruction time.
          \param n size of input array
          \param init allocated input data
        */
       TCArrayWrapper(size_t n, const T *init):
              m_size(n),
              m_cdata(init)
       {
       }


       /**
          Standard array access operator, read-write version
        */
       reference operator[] (size_t i)
       {
              assert(i < m_size);
              DEBUG_ASSERT_RELEASE_THROW(m_data && m_data.unique(),
                                         "TCArrayWrapper::operator[]: No writeable data available or not unique,"
                                         " call TCArrayWrapper::make_unique() first or enforce the use of  "
                                         "'TCArrayWrapper::operator[](...) const'");
              return m_data.get()[i];
       }

       /**
          Standard array access operator, read-only version
        */
       const_reference operator[] (size_t i) const
       {
              assert(i < m_size);
              return m_cdata[i];
       }

       /**
          STL compatible iterator, begin of range
        */
       iterator begin()
       {
              DEBUG_ASSERT_RELEASE_THROW(m_data && m_data.unique(),
                                         "TCArrayWrapper::begin(): No writeable data available or not unique, "
                                         "call TCArrayWrapper::make_unique() first or enforce the use of "
                                         "'TCArrayWrapper::begin() const'");
              return m_data.get();
       }

       /**
          STL compatible iterator, end of range
        */
       iterator end()
       {
              DEBUG_ASSERT_RELEASE_THROW(m_data && m_data.unique(),
                                         "TCArrayWrapper::begin(): No writeable data available or not unique, "
                                         "call TCArrayWrapper::make_unique() first or enforce the use of "
                                         "'TCArrayWrapper::end() const'");
              return m_data.get() + m_size;
       }


       /**
          STL compatible const_iterator, begin of range
        */
       const_iterator begin() const
       {
              return m_cdata;
       }

       /**
          STL compatible const_iterator, end of range
        */
       const_iterator end() const
       {
              return m_cdata + m_size;
       }

       /**
          \returns number of elements in the array
        */
       size_type size() const
       {
              return m_size;
       }

       /**
           If the wrapped data is referenced more than once or is
           read-only, make a copy inside this object that is refereneces
           only once and writable.
       */
       void make_unique()
       {
              // if we have writable dataand is it already unique
              // then do nothing
              if (m_data && m_data.unique())
                     return;

              // create the new array and copy from the constant origial
              // in case we didn't have writable data
              m_data.reset(new T[m_size], array_destructor<T>());
              std::copy(m_cdata, m_cdata + m_size, m_data.get());
              m_cdata = m_data.get();
       }

private:
       size_t m_size;
       std::shared_ptr<T> m_data;
       const T *m_cdata;
};


template <typename T>
std::ostream&  operator << (std::ostream& os, const TCArrayWrapper<T>& v)
{
       os << "[";

       for (auto i : v)
              os << i << ", ";

       os << "]";
       return os;
}


/**
    \ingroup misc

    STL vector like c-array wrapper for double floating point arrays
*/
typedef TCArrayWrapper<double> CDoubleVector;

NS_MIA_END


#endif
