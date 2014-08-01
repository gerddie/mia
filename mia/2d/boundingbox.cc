/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/2d/boundingbox.hh>


NS_MIA_BEGIN

C2DBoundingBox::C2DBoundingBox():
	m_empty(true)
{
}

C2DBoundingBox::C2DBoundingBox(const C2DFVector& begin, const C2DFVector& end):
	m_empty(false),
	m_begin(begin),
	m_end(end)
{
	assert(m_begin.x <= m_end.x);
	assert(m_begin.y <= m_end.y);
}

void C2DBoundingBox::unite(const C2DBoundingBox& other)
{
	if (m_empty) {
		if (other.m_empty)
			return;
		*this = other;
	}
	if (other.m_begin.x < m_begin.x)
		m_begin.x = other.m_begin.x;

	if (other.m_begin.y < m_begin.y)
		m_begin.y = other.m_begin.y;

	if (other.m_end.x > m_end.x)
		m_end.x = other.m_end.x;

	if (other.m_end.y > m_end.y)
		m_end.y = other.m_end.y;
}

C2DFVector C2DBoundingBox::get_begin() const
{
	assert(!m_empty);
	return m_begin;
}

C2DFVector C2DBoundingBox::get_end() const
{
	assert(!m_empty);
	return m_end;
}

C2DFVector C2DBoundingBox::get_size() const
{
	assert(!m_empty);
	return m_end - m_begin;
}

C2DIVector C2DBoundingBox::get_grid_begin() const
{
	assert(!m_empty);
	return C2DIVector((int)floor(m_begin.x),
			  (int)floor(m_begin.y));
}

C2DIVector C2DBoundingBox::get_grid_end() const
{
	assert(!m_empty);
 	return C2DIVector((int)ceil(m_end.x),
			  (int)ceil(m_end.y));

}

C2DBounds C2DBoundingBox::get_grid_size() const
{
	assert(!m_empty);
	return C2DBounds(get_grid_end() - get_grid_begin());
}

void C2DBoundingBox::add(const C2DFVector& point)
{
	if (m_empty) {
		m_begin = m_end = point;
		m_empty = false;
		return;
	}
	if (point.x < m_begin.x)
		m_begin.x = point.x;

	if (point.y < m_begin.y)
		m_begin.y = point.y;

	if (point.x > m_end.x)
		m_end.x = point.x;

	if (point.y > m_end.y)
		m_end.y = point.y;

}

void C2DBoundingBox::enlarge(float boundary)
{
	m_end.y += boundary;
	m_end.x += boundary;
	m_begin.y -= boundary;
	m_begin.x -= boundary;

}

bool C2DBoundingBox::empty() const
{
	return m_empty;
}

NS_MIA_END

