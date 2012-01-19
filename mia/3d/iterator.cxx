/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
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
	m_ystride(0), 
	m_boundary(eb_none)
{
}

template <typename I> 
range3d_iterator<I>::range3d_iterator(const C3DBounds& pos):
	m_pos(pos), 
	m_size(0,0,0), 
	m_begin(0,0,0), 
	m_end(0,0,0), 
	m_xstride(0),
	m_ystride(0), 
	m_boundary(eb_none)
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
	m_iterator(iterator), 
	m_boundary(eb_none)
{
	cvdebug() << "m_boundary=" << m_boundary << "\n"; 
	m_ystride = (m_size.y - (m_end.y - m_begin.y))*m_size.x; 
	if (m_pos.x == 0)
		m_boundary |= eb_xlow; 
	if (m_pos.x == size.x - 1)
		m_boundary |= eb_xhigh; 
	
	if (m_pos.y == 0)
		m_boundary |= eb_ylow; 
	if (m_pos.y == size.y - 1)
		m_boundary |= eb_yhigh; 

	if (m_pos.z == 0)
		m_boundary |= eb_zlow; 
	if (m_pos.z == size.z - 1)
		m_boundary |= eb_zhigh; 
	cvdebug() << "m_boundary=" << m_boundary << "\n"; 
	
}


template <typename I> 
range3d_iterator<I>& range3d_iterator<I>::operator = (const range3d_iterator<I>& other)
{
	m_pos = other.m_pos; 
	m_size = other.m_size;  
	m_begin = other.m_begin; 
	m_end = other.m_end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_ystride = other.m_ystride;
	m_boundary = other.m_boundary; 
	return *this; 
}



template <typename I> 
range3d_iterator<I>::range3d_iterator(const range3d_iterator<I>& other):
	m_pos(other.m_pos), 
	m_size(other.m_size), 
	m_begin(other.m_begin), 
	m_end(other.m_end), 
	m_xstride(other.m_xstride),
	m_ystride(other.m_ystride),
	m_iterator(other.m_iterator), 
	m_boundary(other.m_boundary)
{
}	

template <typename I> 
range3d_iterator<I>& range3d_iterator<I>::operator ++()
{
	DEBUG_ASSERT_RELEASE_THROW(m_pos.x < m_end.x, "range3d_iterator: trying to increment past end");
	
	++m_pos.x;
	++m_iterator; 
	m_boundary &= ~eb_x;
	if (m_pos.x == m_end.x) {
		increment_y(); 
		if (m_pos.x == 0) 
			m_boundary |= eb_xlow;
	}else if (m_pos.x == m_size.x - 1)
		m_boundary |= eb_xhigh; 
	
	return *this; 
}

template <typename I> 
void range3d_iterator<I>::increment_y()
{
	if (m_pos.y < m_end.y) {
		m_pos.x = m_begin.x; 
		++m_pos.y; 
		m_boundary &= ~eb_y;
		std::advance(m_iterator, m_xstride);
		if (m_pos.y == m_end.y) {
			increment_z();
			if (m_pos.y == 0) 
				m_boundary |= eb_ylow; 
		} else if (m_pos.y == m_size.y - 1)
			m_boundary |= eb_yhigh; 
	} 
}

template <typename I> 
void range3d_iterator<I>::increment_z()
{
	if (m_pos.z < m_end.z - 1) {
		m_pos.y = m_begin.y; 
		++m_pos.z; 
		m_boundary &= ~eb_zlow; 
		std::advance(m_iterator, m_ystride);
		if (m_pos.z == m_size.z - 1)
			m_boundary |= eb_zhigh; 
	}else if (m_pos.z == m_end.z - 1) {
		m_pos = m_end; 

	}
}

template <typename I> 
typename range3d_iterator<I>::internal_iterator 
range3d_iterator<I>::get_point()
{
	return m_iterator; 
}

template <typename I> 
range3d_iterator<I> range3d_iterator<I>::operator ++(int)
{
	range3d_iterator result(*this); 
	++result; 
	return result; 
}

template <typename I> 
typename range3d_iterator<I>::reference range3d_iterator<I>::operator *() const
{
	return *m_iterator; 
}

template <typename I> 
struct __iterator3d_dispatch_operator_for_bool {
	static typename range3d_iterator<I>::pointer 
	apply(const typename range3d_iterator<I>::internal_iterator& it) {
		return it.operator->(); 
	}
}; 

template <> 
struct __iterator3d_dispatch_operator_for_bool<std::vector<bool>::iterator> {
	static std::vector<bool>::iterator::pointer apply(const std::vector<bool>::iterator& ) {
		throw std::logic_error("You can't use the '->' with a bool 3DDatafield range iterator"); 
	}
}; 

template <> 
struct __iterator3d_dispatch_operator_for_bool<std::vector<bool>::const_iterator> {
	static std::vector<bool>::const_iterator::pointer apply(const std::vector<bool>::const_iterator& ) {
		throw std::logic_error("You can't use the '->' with a bool 3DDatafield range iterator"); 
	}
}; 
			

template <typename I> 
typename range3d_iterator<I>::pointer range3d_iterator<I>::operator ->() const
{
	return __iterator3d_dispatch_operator_for_bool<internal_iterator>::apply(m_iterator);
}

template <typename I> 
const C3DBounds& range3d_iterator<I>::pos() const
{
	return m_pos; 
}

template <typename I> 
int range3d_iterator<I>::get_boundary_flags() const
{
	return m_boundary; 
}

NS_MIA_END
