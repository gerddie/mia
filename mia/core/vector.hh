/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


/**
   \cond INTERNAL
   Helper structure used to initialize std::shared_ptr with the proper 
   delete operator.  
 */
template <typename T> 
struct array_destructor {
	/// run delete on an array 
	virtual void operator () (T *p) {
		delete[] p; 
	}
}; 

/**
   Helper structure used to initialize std::shared_ptr with an empty 
   delete operator, i.e. to fake a shared pointer 
 */
template <typename T> 
struct array_void_destructor {
	/// skip deleting the pointer 
	virtual void operator () (T *) {
	}
}; 
/// \endcond 

/**
   \ingroup templates
   \brief A wrapper around the c-array to provide an STL like interface for iterators 

   c-array envelope that supports some facilities of STL like vectors and that 
   allows holding pre-allocated data. 
   Handling of the optinal deleting of the array is done by a destructor template
   passed to the std::shared_ptr constructor 
   \tparam the data type of the C-array 
*/

template <typename T> 
class Vector {
public: 
	
	/// \cond STLCOMPAT 
	typedef T& reference;  
	typedef const T& const_reference;  
	typedef T *iterator;  
	typedef const T *const_iterator;  
	typedef size_t size_type; 
	/// \endcond 

	/**
	   Create a vector, the data is owned by this vector and will be 
	   deleted if the reference count reaches zero 
	   \param n 
	   \param clean initialize vector to 0
	 */
	Vector(size_t n, bool clean = true):
		m_size(n),
		m_data(new T[n], array_destructor<T>()),
		m_cdata(m_data.get())
	{
		if (clean) 
			memset(m_data.get(), 0, m_size*sizeof(T)); 
	}

	/** copy constructor, this is a shallow copy, i.e. the data is shared 
	    between the original and the copied vector 
	*/
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
	

	/**
	   Standard array access operator, read-write version 
	 */
	reference operator[] (size_t i) {
		assert(i < m_size); 
		assert(m_data); 
		assert(m_data.unique()); 
		return m_data.get()[i]; 
	}

	/**
	   Standard array access operator, read-only version 
	 */
	const_reference operator[] (size_t i) const {
		assert(i < m_size); 
		return m_cdata[i]; 
	}
	
	/**
	   STL compatible iterator, begin of range  
	 */
	iterator begin() {
		assert(m_data); 
		assert(m_data.unique()); 
		return m_data.get(); 
	}

	/**
	   STL compatible iterator, end of range  
	 */
	iterator end() {
		assert(m_data); 
		return m_data.get() + m_size; 
	}
	

	/**
	   STL compatible const_iterator, begin of range   
	 */
	const_iterator begin() const{
		return m_cdata; 
	}

	/**
	   STL compatible const_iterator, end of range   
	 */
	const_iterator end() const{
		return m_cdata + m_size; 
	}
	
	/**
	   \returns number of elements in the array 
	 */
	size_type size() const 
	{
		return m_size; 
	}

private: 
	size_t m_size; 
	std::shared_ptr<T> m_data; 
	const T *m_cdata; 
}; 


/// STL like c-array wrapper for double floating point
typedef Vector<double> CDoubleVector; 

NS_MIA_END


#endif
