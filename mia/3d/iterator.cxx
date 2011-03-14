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

#include <mia/core/errormacro.hh>
#include <mia/3d/iterator.hh>

NS_MIA_BEGIN

template <typename I> 
range3d_iterator<I>::range3d_iterator():
	m_pos(0,0,0), 
	m_size(0,0,0), 
	m_begin(0,0,0), 
	m_end(0,0,0), 
	m_xstride(0),
	m_ystride(0)
{
}

template <typename I> 
range3d_iterator<I>::range3d_iterator(const C3DBounds& pos):
	m_pos(pos), 
	m_size(0,0,0), 
	m_begin(0,0,0), 
	m_end(0,0,0), 
	m_xstride(0),
	m_ystride(0)
{
}


template <typename I> 
range3d_iterator<I>::range3d_iterator(const C3DBounds& pos, const C3DBounds& size, 
			       const C3DBounds& begin, const C3DBounds& end, I iterator):
	m_pos(pos), 
	m_size(size), 
	m_begin(begin), 
	m_end(end),
	m_xstride(m_size.x - (m_end.x - m_begin.x)),
	m_iterator(iterator)
{
	m_ystride = (m_size.y - (m_end.y - m_begin.y))*m_size.x; 
}

template <typename I> 
range3d_iterator<I>& range3d_iterator<I>::operator = (const range3d_iterator<I>& other)
{
	m_pos = other.pos; 
	m_size = other.size;  
	m_begin = other.begin; 
	m_end = other.end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_ystride = other.m_ystride; 
}

template <typename I> 
range3d_iterator<I>::range3d_iterator(const range3d_iterator<I>& other):
	m_pos(other.pos), 
	m_size(other.size), 
	m_begin(other.begin), 
	m_end(other.end), 
	m_iterator(other.m_iterator), 
	m_xstride(other.m_xstride),
	m_ystride(other.m_ystride)
{
}	

template <typename I> 
range3d_iterator<I>& range3d_iterator<I>::operator ++()
{
	DEBUG_ASSERT_RELEASE_THROW(m_pos.x < m_end.x, "range3d_iterator: trying to increment past end");
	
	++m_pos.x;
	++m_iterator; 
	if (m_pos.x == m_end.x)
		increment_y(); 
	return *this; 
}

template <typename I> 
void range3d_iterator<I>::increment_y()
{
	if (m_pos.y < m_end.y) {
		m_pos.x = m_begin.x; 
		++m_pos.y; 
		std::advance(m_iterator, m_xstride);
		if (m_pos.y == m_end.y) 
			increment_z();
	} 
}

template <typename I> 
void range3d_iterator<I>::increment_z()
{
	if (m_pos.z < m_end.z - 1) {
		m_pos.y = m_begin.y; 
		++m_pos.z; 
		std::advance(m_iterator, m_ystride);
	}else if (m_pos.z == m_end.z - 1) {
		m_pos = m_end; 
	}
}

template <typename I> 
range3d_iterator<I> range3d_iterator<I>::operator ++(int)
{
	range3d_iterator result(*this); 
	++result; 
	return result; 
}

template <typename I> 
typename I::value_type& range3d_iterator<I>::operator *() const
{
	return *m_iterator; 
}

template <typename I> 
typename I::value_type  *range3d_iterator<I>::operator ->() const
{
	return m_iterator; 
}

template <typename I> 
const C3DBounds& range3d_iterator<I>::pos() const
{
	return m_pos; 
}

NS_MIA_END
