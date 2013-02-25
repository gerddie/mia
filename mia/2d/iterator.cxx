/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <stdexcept>
#include <mia/core/errormacro.hh>
#include <mia/2d/iterator.hh>

NS_MIA_BEGIN

template <typename I> 
range2d_iterator<I>::range2d_iterator():
	m_pos(0,0), 
	m_size(0,0), 
	m_begin(0,0), 
	m_end(0,0), 
	m_xstride(0),
	m_boundary(eb_none)
{
}

template <typename I> 
range2d_iterator<I>::range2d_iterator(const C2DBounds& pos):
	m_pos(pos), 
	m_size(0,0), 
	m_begin(0,0), 
	m_end(0,0), 
	m_xstride(0),
	m_boundary(eb_none)
{
}

template <typename I> 
range2d_iterator<I>::range2d_iterator(const C2DBounds& pos, const C2DBounds& size, 
				      const C2DBounds& begin, const C2DBounds& end, I iterator):
	m_pos(pos), 
	m_size(size), 
	m_begin(begin), 
	m_end(end),
	m_xstride(m_size.x - (m_end.x - m_begin.x)),
	m_iterator(iterator), 
	m_boundary(eb_none)
{
	cvdebug() << "m_boundary=" << m_boundary << "\n"; 
	if (m_pos.x == 0)
		m_boundary |= eb_xlow; 
	if (m_pos.x == size.x - 1)
		m_boundary |= eb_xhigh; 
	
	if (m_pos.y == 0)
		m_boundary |= eb_ylow; 
	if (m_pos.y == size.y - 1)
		m_boundary |= eb_yhigh; 

	cvdebug() << "m_boundary=" << m_boundary << "\n"; 
	
}


template <typename I> 
range2d_iterator<I>& range2d_iterator<I>::operator = (const range2d_iterator<I>& other)
{
	m_pos = other.m_pos; 
	m_size = other.m_size;  
	m_begin = other.m_begin; 
	m_end = other.m_end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_boundary = other.m_boundary; 
	return *this; 
}



template <typename I> 
range2d_iterator<I>::range2d_iterator(const range2d_iterator<I>& other):
	m_pos(other.m_pos), 
	m_size(other.m_size), 
	m_begin(other.m_begin), 
	m_end(other.m_end), 
	m_xstride(other.m_xstride),
	m_iterator(other.m_iterator), 
	m_boundary(other.m_boundary)
{
}	

template <typename I> 
range2d_iterator<I>& range2d_iterator<I>::operator ++()
{
	DEBUG_ASSERT_RELEASE_THROW(m_pos.x < m_end.x, "range2d_iterator: trying to increment past end");
	
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
void range2d_iterator<I>::increment_y()
{
	if (m_pos.y < m_end.y - 1) {
		m_pos.x = m_begin.x; 
		++m_pos.y; 
		m_boundary &= ~eb_ylow; 
		std::advance(m_iterator, m_xstride);
		if (m_pos.y == m_size.y - 1)
			m_boundary |= eb_yhigh; 
	}else if (m_pos.y == m_end.y - 1) {
		m_pos = m_end; 

	}
}

template <typename I> 
typename range2d_iterator<I>::internal_iterator 
range2d_iterator<I>::get_point()
{
	return m_iterator; 
}

template <typename I> 
range2d_iterator<I> range2d_iterator<I>::operator ++(int)
{
	range2d_iterator result(*this); 
	++(*this); 
	return result; 
}

template <typename I> 
typename range2d_iterator<I>::reference range2d_iterator<I>::operator *() const
{
	return *m_iterator; 
}

template <typename I> 
struct __iterator2d_dispatch_operator_for_bool {
	static typename range2d_iterator<I>::pointer 
	apply(const typename range2d_iterator<I>::internal_iterator& it) {
		return it.operator->(); 
	}
}; 

template <> 
struct __iterator2d_dispatch_operator_for_bool<std::vector<bool>::iterator> {
	static std::vector<bool>::iterator::pointer apply(const std::vector<bool>::iterator& ) {
		throw std::logic_error("You can't use the '->' with a bool 2DDatafield range iterator"); 
	}
}; 

template <> 
struct __iterator2d_dispatch_operator_for_bool<std::vector<bool>::const_iterator> {
	static std::vector<bool>::const_iterator::pointer apply(const std::vector<bool>::const_iterator& ) {
		throw std::logic_error("You can't use the '->' with a bool 2DDatafield range iterator"); 
	}
}; 
			

template <typename I> 
typename range2d_iterator<I>::pointer range2d_iterator<I>::operator ->() const
{
	return __iterator2d_dispatch_operator_for_bool<internal_iterator>::apply(m_iterator);
}

template <typename I> 
const C2DBounds& range2d_iterator<I>::pos() const
{
	return m_pos; 
}

template <typename I> 
int range2d_iterator<I>::get_boundary_flags() const
{
	return m_boundary; 
}

NS_MIA_END
