/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <mia/2d/transform.hh>
#include <mia/2d/deformer.hh>


#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
using namespace boost::lambda;

C2DTransformation::C2DTransformation()
{

}

void C2DTransformation::reinit() const
{
}

void C2DTransformation::set_creator_string(const std::string& s)
{
	_M_creator_string = s; 
}


const std::string& C2DTransformation::get_creator_string()const
{
	return _M_creator_string; 
}

C2DTransformation *C2DTransformation::clone() const
{
	C2DTransformation *result = do_clone(); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}


C2DTransformation::iterator_impl::iterator_impl():
	_M_pos(0,0), 
	_M_size(0,0)
{
}

C2DTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size):
	_M_pos(pos), 
	_M_size(size)
{
}

void C2DTransformation::iterator_impl::increment()
{
	++_M_pos.x;
	if (_M_pos.x < _M_size.x)
		do_x_increment();
	else {
		if (_M_pos.y < _M_size.y) {
			++_M_pos.y;
			if (_M_pos.y < _M_size.y)
				_M_pos.x = 0;
			else 
				return ; 
			do_y_increment();
		}
	}
}

const C2DBounds& C2DTransformation::iterator_impl::get_pos()const
{
	return _M_pos; 
}

const C2DBounds& C2DTransformation::iterator_impl::get_size()const
{
	return _M_size; 
}


const C2DFVector&  C2DTransformation::iterator_impl::get_value() const
{
	return do_get_value(); 
}

bool C2DTransformation::iterator_impl::operator == (const C2DTransformation::iterator_impl& b) const
{
	assert(_M_size == b._M_size); 
	return _M_pos == b._M_pos; 
}

EXPORT_2D bool operator == (const C2DTransformation::const_iterator& a, 
			    const C2DTransformation::const_iterator& b)
{
	return *a._M_holder == *b._M_holder; 
}


EXPORT_2D bool operator != (const C2DTransformation::const_iterator& a, 
			    const C2DTransformation::const_iterator& b)
{
	return !(a == b); 
}


C2DTransformation::const_iterator::const_iterator():
	_M_holder(NULL)
{
}

C2DTransformation::const_iterator::const_iterator(iterator_impl * holder):
	_M_holder(holder)
{
}

C2DTransformation::const_iterator::const_iterator(const const_iterator& other):
	_M_holder(other._M_holder->clone())
{
}

C2DTransformation::const_iterator& 
C2DTransformation::const_iterator::operator = (const const_iterator& other)
{
	_M_holder.reset(other._M_holder->clone()); 
	return *this; 
}

C2DTransformation::const_iterator& C2DTransformation::const_iterator::operator ++()
{
	_M_holder->increment(); 
	return *this; 
}

C2DTransformation::const_iterator C2DTransformation::const_iterator::operator ++(int)
{
	auto old = _M_holder->clone(); 
	++(*this); 
	return C2DTransformation::const_iterator(old); 
}

const C2DFVector&  C2DTransformation::const_iterator::operator *() const
{
	assert(_M_holder); 
	return _M_holder->get_value(); 
}

const C2DFVector  *C2DTransformation::const_iterator::operator ->() const
{
	assert(_M_holder); 
	return &_M_holder->get_value(); 
}


NS_MIA_END
