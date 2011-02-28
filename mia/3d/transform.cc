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

#include <mia/3d/transform.hh>
#include <mia/3d/deformer.hh>


#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
using namespace boost::lambda;

C3DTransformation::C3DTransformation()
{

}

void C3DTransformation::reinit() const
{
}

void C3DTransformation::set_creator_string(const std::string& s)
{
	_M_creator_string = s; 
}


const std::string& C3DTransformation::get_creator_string()const
{
	return _M_creator_string; 
}

C3DTransformation *C3DTransformation::clone() const
{
	C3DTransformation *result = do_clone(); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}

P3DTransformation C3DTransformation::upscale(const C3DBounds& size) const
{
	P3DTransformation result = do_upscale(size); 
	if (result) 
		result->set_creator_string(get_creator_string()); 
	return result; 
}



C3DTransformation::iterator_impl::iterator_impl():
	_M_pos(0,0,0), 
	_M_size(0,0,0)
{
}

C3DTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size):
	_M_pos(pos), 
	_M_size(size)
{
}

void C3DTransformation::iterator_impl::increment()
{

	if (_M_pos.x < _M_size.x) {
		++_M_pos.x;
		if (_M_pos.x < _M_size.x) {
			do_x_increment();
			return; 
		}
	} else {
		// if x is at the end, then everything is at the end
		cvwarn() << "C3DTransformation::iterator_impl::increment() past end\n"; 
		return; 
	}
	
	++_M_pos.y;
	if (_M_pos.y < _M_size.y) {
		_M_pos.x = 0;
		do_y_increment();
		return; 
	}
	
	++_M_pos.z; 
	if (_M_pos.z < _M_size.z) {
		_M_pos.y = 0;
		_M_pos.x = 0;
		do_z_increment();
		return; 
	}
}

const C3DBounds& C3DTransformation::iterator_impl::get_pos()const
{
	return _M_pos; 
}

const C3DBounds& C3DTransformation::iterator_impl::get_size()const
{
	return _M_size; 
}


const C3DFVector&  C3DTransformation::iterator_impl::get_value() const
{
	return do_get_value(); 
}

bool C3DTransformation::iterator_impl::operator == (const C3DTransformation::iterator_impl& b) const
{
	assert(_M_size == b._M_size); 
	return _M_pos == b._M_pos; 
}

EXPORT_3D bool operator == (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b)
{
	return *a._M_holder == *b._M_holder; 
}


EXPORT_3D bool operator != (const C3DTransformation::const_iterator& a, 
			    const C3DTransformation::const_iterator& b)
{
	return !(a == b); 
}


C3DTransformation::const_iterator::const_iterator():
	_M_holder(NULL)
{
}

C3DTransformation::const_iterator::const_iterator(iterator_impl * holder):
	_M_holder(holder)
{
}

C3DTransformation::const_iterator::const_iterator(const const_iterator& other):
	_M_holder(other._M_holder->clone())
{
}

C3DTransformation::const_iterator& 
C3DTransformation::const_iterator::operator = (const const_iterator& other)
{
	_M_holder.reset(other._M_holder->clone()); 
	return *this; 
}

C3DTransformation::const_iterator& C3DTransformation::const_iterator::operator ++()
{
	_M_holder->increment(); 
	return *this; 
}

C3DTransformation::const_iterator C3DTransformation::const_iterator::operator ++(int)
{
	auto old = _M_holder->clone(); 
	++(*this); 
	return C3DTransformation::const_iterator(old); 
}

const C3DFVector&  C3DTransformation::const_iterator::operator *() const
{
	assert(_M_holder); 
	return _M_holder->get_value(); 
}

const C3DFVector  *C3DTransformation::const_iterator::operator ->() const
{
	assert(_M_holder); 
	return &_M_holder->get_value(); 
}

bool C3DTransformation::refine()
{
	return false; 
}

const char *C3DTransformation::type_descr = "3dtransform";
const char *C3DTransformation::dim_descr = "3d";

NS_MIA_END
