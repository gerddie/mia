/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <mia/2d/BoundingBox.hh>


NS_MIA_BEGIN

C2DBoundingBox::C2DBoundingBox():
	_M_empty(true)
{
}

C2DBoundingBox::C2DBoundingBox(const C2DFVector& begin, const C2DFVector& end):
	_M_empty(false),
	_M_begin(begin),
	_M_end(end)
{
	assert(_M_begin.x <= _M_end.x);
	assert(_M_begin.y <= _M_end.y);
}

void C2DBoundingBox::unite(const C2DBoundingBox& other)
{
	if (_M_empty) {
		if (other._M_empty)
			return;
		*this = other;
	}
	if (other._M_begin.x < _M_begin.x)
		_M_begin.x = other._M_begin.x;

	if (other._M_begin.y < _M_begin.y)
		_M_begin.y = other._M_begin.y;

	if (other._M_end.x > _M_end.x)
		_M_end.x = other._M_end.x;

	if (other._M_end.y > _M_end.y)
		_M_end.y = other._M_end.y;
}

C2DFVector C2DBoundingBox::get_begin() const
{
	assert(!_M_empty);
	return _M_begin;
}

C2DFVector C2DBoundingBox::get_end() const
{
	assert(!_M_empty);
	return _M_end;
}

C2DFVector C2DBoundingBox::get_size() const
{
	assert(!_M_empty);
	return _M_end - _M_begin;
}

C2DIVector C2DBoundingBox::get_grid_begin() const
{
	assert(!_M_empty);
	return C2DIVector((int)floor(_M_begin.x),
			  (int)floor(_M_begin.y));
}

C2DIVector C2DBoundingBox::get_grid_end() const
{
	assert(!_M_empty);
 	return C2DIVector((int)ceil(_M_end.x),
			  (int)ceil(_M_end.y));

}

C2DBounds C2DBoundingBox::get_grid_size() const
{
	assert(!_M_empty);
	return C2DBounds(get_grid_end() - get_grid_begin());
}

void C2DBoundingBox::add(const C2DFVector& point)
{
	if (_M_empty) {
		_M_begin = _M_end = point;
		_M_empty = false;
		return;
	}
	if (point.x < _M_begin.x)
		_M_begin.x = point.x;

	if (point.y < _M_begin.y)
		_M_begin.y = point.y;

	if (point.x > _M_end.x)
		_M_end.x = point.x;

	if (point.y > _M_end.y)
		_M_end.y = point.y;

}

void C2DBoundingBox::enlarge(float boundary)
{
	_M_end.y += boundary;
	_M_end.x += boundary;
	_M_begin.y -= boundary;
	_M_begin.x -= boundary;

}

bool C2DBoundingBox::empty() const
{
	return _M_empty;
}

NS_MIA_END

