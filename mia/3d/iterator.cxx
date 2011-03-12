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


#include <mia/3d/iterator.hh>

NS_MIA_BEGIN

template <typename I> 
range3d_iterator<I>::range3d_iterator():
	m_pos(0,0,0), 
	m_size(0,0,0), 
	m_start(0,0,0), 
	m_end(0,0,0), 
	m_xstride(0),
	m_ystride(0)
{
}

template <typename I> 
range3d_iterator<I>::range3d_iterator(const C3DBounds& pos, const C3DBounds& size, 
			       const C3DBounds& start, const C3DBounds& end, I iterator):
	m_pos(pos), 
	m_size(size), 
	m_start(start), 
	m_end(end),
	m_iterator(iterator), 
	m_xstride(m_size.x - (m_end.x - m_begin.x)),
	m_ystride((m_size.y - (m_end.y - m_begin.y))*m_size.x + m_xstride)
{
}

template <typename I> 
range3d_iterator<I>& range3d_iterator<T>::operator = (const range3d_iterator<T>& other)
{
	m_pos = other.pos; 
	m_size = other.size;  
	m_start = other.start; 
	m_end = other.end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_ystride = other.m_ystride; 
}

template <typename I> 
range3d_iterator<I>::range3d_iterator(const range3d_iterator& other):
	m_pos(other.pos), 
	m_size(other.size), 
	m_start(other.start), 
	m_end(other.end), 
	m_iterator(other.m_iterator), 
	m_xstride(other.m_xstride),
	m_ystride(other.m_ystride)
{
}	

template <typename I> 
range3d_iterator<I>& range3d_iterator::operator ++()
{
	DEBUG_ASSERT_RELEASE_THROW(m_pos.x >= m_end.x);
	
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
		m_pos.x = m_start.x; 
		++m_pos.y; 
		if (m_pos.y == m_end.y)
			increment_z(); 
		else
			std::advance(m_iterator, m_xstride);
	}
}

template <typename I> 
void range3d_iterator<I>::increment_z()
{
	if (m_pos.z < m_end.z) {
		m_pos.y = m_start.y; 
		++m_pos.z; 
		std::advance(m_iterator, m_ystride);
	}
}

template <typename I> 
range3d_iterator<I> range3d_iterator::operator ++(int)
{
	range3d_iterator result(*this); 
	++result; 
	return result; 
}

template <typename I> 
typename range3d_iterator<I>::value_type& range3d_iterator<T>::operator *() const
{
	return *iterator; 
}

template <typename I> 
typename range3d_iterator<I>::value_type  *range3d_iterator<T>::operator ->() const;
{
	return iterator; 
}

template <typename I> 
const C3DBounds& range3d_iterator<I>::pos() const
{
	return m_pos; 
}

template <typename I> 
bool range3d_iterator<I>::operator == (const range3d_iterator<I>& other)const
{
	// we really want these two to the same range 
	assert(m_size == other.m_size);
	assert(m_start == other.m_start);
	assert(m_end == other.m_end);

	return m_pos == other.m_pos; 
}

NS_MIA_END
