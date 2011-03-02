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

NS_MIA_BEGIN

template <typename T> 
struct array_destructor {
	virtual void operator () (T *p) {
		delete[] p; 
	}
}; 

template <typename T> 
struct array_void_destructor : public array_destructor<T> {
	virtual void operator () (T *) {
	}
}; 


template <typename T> 
class Vector {
public: 
	typedef T& reference;  
	typedef const T& const_reference;  
	typedef T *iterator;  
	typedef T *const_iterator;  
	typedef size_t size_type; 

	Vector(size_t n):
		m_size(n),
		m_data(new T[n], array_destructor<T>())
	{
	}

	Vector(const Vector& other):
		m_size(other.m_size),
		m_data(other.m_data)
	{
	}

	Vector<T>& operator = (const Vector& other)
	{
		m_size = other.m_size; 
		m_data = other.m_data; 
		return *this; 
	}

	Vector(size_t n, T *init):
		m_size(n),
		m_data(init, array_void_destructor<T>())
	{
	}
		
	reference operator[] (size_t i) {
		assert(i < m_size); 
		return m_data.get()[i]; 
	}

	const_reference operator[] (size_t i) const {
		assert(i < m_size); 
		return m_data.get()[i]; 
	}
	
	iterator begin() {
		return m_data.get(); 
	}

	iterator end() {
		return m_data.get() + m_size; 
	}
	
	const_iterator begin() const{
		return m_data.get(); 
	}

	const_iterator end() const{
		return m_data.get() + m_size; 
	}
	
private: 
	size_t m_size; 
	std::shared_ptr<T> m_data; 
}; 

typedef Vector<double> DoubleVector; 

NS_MIA_END


#endif
