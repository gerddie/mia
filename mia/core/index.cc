/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/core/index.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

CCircularIndex::CCircularIndex(unsigned int size, unsigned int start):
	m_size(size), 
	m_start(start), 
	m_fill(0), 
	m_next_fill(0), 
	m_cur_start(0)
{
}
	
void CCircularIndex::insert_one()
{
	DEBUG_ASSERT_RELEASE_THROW(m_fill < m_size, "buffer is full, can no longer insert."); 
	
	++m_fill; 
	++m_next_fill; 
	if (m_next_fill >= m_size)
		m_next_fill = 0; 
}

unsigned int CCircularIndex::next() const
{
	return m_next_fill; 
}

void CCircularIndex::new_start(unsigned int start)
{
	if (start != m_start) {
		int delta = start - m_start; 
		m_fill -= delta; 
		m_cur_start += delta; 
		if (m_cur_start >= m_size) 
			m_cur_start -= m_size; 
		m_start = start; 
	}
}

unsigned int CCircularIndex::fill()const
{
	return m_fill; 
}

unsigned int CCircularIndex::value(unsigned int i) const
{
	return (i + m_cur_start) % m_size; 
}

NS_MIA_END
