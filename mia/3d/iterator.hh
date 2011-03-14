/* -*- mia-c++  -*-
 *   
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#ifndef mia_3d_iterator_hh
#define mia_3d_iterator_hh

#include <mia/3d/3DVector.hh>

NS_MIA_BEGIN

template <typename I> 
class range3d_iterator: public std::forward_iterator_tag {
public: 
	typedef typename I::reference reference; 
	typedef typename I::pointer pointer; 
	typedef I internal_iterator; 
	
	range3d_iterator(); 
	range3d_iterator(const C3DBounds& pos, const C3DBounds& size, 
			 const C3DBounds& start, const C3DBounds& end, I iterator);
	
	/**
	   End iterator, can't be dereferenced  
	 */
	range3d_iterator(const C3DBounds& pos);


	template <typename AI> 
	friend class range3d_iterator; 
	
	template <typename AI>
	range3d_iterator<I>& operator = (const range3d_iterator<AI>& other); 

	
	template <typename AI>
	range3d_iterator(const range3d_iterator<AI>& other); 

	range3d_iterator<I>& operator = (const range3d_iterator<I>& other); 
	range3d_iterator(const range3d_iterator<I>& other); 
	
	range3d_iterator<I>& operator ++(); 
	range3d_iterator<I> operator ++(int); 
	
	reference  operator *() const;
	pointer    operator ->() const;
	
	const C3DBounds& pos() const; 

	template <typename T> friend
	bool operator == (const range3d_iterator<T>& left, const range3d_iterator<T>& right); 

private: 

	void increment_y(); 
	void increment_z(); 

	C3DBounds m_pos; 
	C3DBounds m_size; 
	C3DBounds m_begin; 
	C3DBounds m_end; 
	int m_xstride; 
	int m_ystride; 
	I m_iterator; 
}; 



template <typename I> 
template <typename AI>
range3d_iterator<I>& range3d_iterator<I>::operator = (const range3d_iterator<AI>& other)
{
	m_pos = other.m_pos; 
	m_size = other.m_size;  
	m_begin = other.m_begin; 
	m_end = other.m_end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_ystride = other.m_ystride; 
	return *this; 
}

template <typename I> 
template <typename AI>
range3d_iterator<I>::range3d_iterator(const range3d_iterator<AI>& other):
	m_pos(other.m_pos), 
	m_size(other.m_size), 
	m_begin(other.m_begin), 
	m_end(other.m_end), 
	m_xstride(other.m_xstride),
	m_ystride(other.m_ystride),
	m_iterator(other.m_iterator)
{
}	

template <typename I> 
bool operator == (const range3d_iterator<I>& left, const range3d_iterator<I>& right)
{
	// we really want these two to the same range 
//	assert(left.m_size == right.m_size);
//	assert(left.m_begin == right.m_begin);
//	assert(left.m_end == right.m_end);

	return left.m_pos == right.m_pos; 

}

template <typename I> 
bool operator != (const range3d_iterator<I>& a, const range3d_iterator<I>& b)
{
	return !(a == b); 
}

NS_MIA_END

#endif
