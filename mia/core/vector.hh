/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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


#ifndef mia_core_vector_hh
#define mia_core_vector_hh

#include <mia/core/defines.hh>
#include <memory>
#include <cstring>
#include <cassert>

NS_MIA_BEGIN

template <typename T> 
struct array_destructor {
	virtual void operator () (T *p) {
		delete[] p; 
	}
}; 

template <typename T> 
struct array_void_destructor {
	virtual void operator () (T *) {
	}
}; 

/**
   c-array envelope that supports some facilities of STL like vectors and that 
   allows holding pre-allocated data. 
   Handling of the optinal deleting of the array is done by a destructor template
   passed to the shared_ptr constructor 
   \tparam the data type of the C-array 
*/


template <typename T> 
class Vector {
public: 
	typedef T& reference;  
	typedef const T& const_reference;  
	typedef T *iterator;  
	typedef const T *const_iterator;  
	typedef size_t size_type; 

	/**
	   Create a vector, the data is won by this vector and will be 
	   deleted if the reference count reaches zero 
	   \param n 
	   \param clean initialize vector to 0
	 */
	Vector(size_t n, bool clean = 0):
		m_size(n),
		m_data(new T[n], array_destructor<T>()),
		m_cdata(m_data.get())
	{
		if (clean) 
			memset(m_data.get(), 0, m_size*sizeof(T)); 
	}

	/// copy constructor 
	Vector(const Vector<T>& other):
		m_size(other.m_size),
		m_data(other.m_data),
		m_cdata(other.m_cdata)
	{
	}

	/// assignment operator 
	Vector<T>& operator = (const Vector<T>& other)
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
	Vector(size_t n, T *init):
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
	Vector(size_t n, const T *init):
		m_size(n),
		m_cdata(init)
	{
	}
	
	reference operator[] (size_t i) {
		assert(i < m_size); 
		assert(m_data); 
		return m_data.get()[i]; 
	}

	const_reference operator[] (size_t i) const {
		assert(i < m_size); 
		return m_cdata[i]; 
	}
	
	iterator begin() {
		assert(m_data); 
		return m_data.get(); 
	}

	iterator end() {
		assert(m_data); 
		return m_data.get() + m_size; 
	}
	
	const_iterator begin() const{
		return m_cdata; 
	}

	const_iterator end() const{
		return m_cdata + m_size; 
	}
	
	size_type size() const 
	{
		return m_size; 
	}

private: 
	size_t m_size; 
	std::shared_ptr<T> m_data; 
	const T *m_cdata; 
}; 

typedef Vector<double> CDoubleVector; 

NS_MIA_END


#endif
