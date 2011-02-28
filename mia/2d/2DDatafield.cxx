/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

// $Id: 2DDatafield.hh 732 2005-11-14 18:06:39Z write1 $

/*! \brief A templated 2D datafield class

The class holds all types of data stored in two dimensional fields.

\file 2DDatafield.hh
\author Gert Wollny <wollny@cbs.mpg.de>, 2004

*/
// implementation part is here:

#ifndef mia_2d_datafield_cxx
#define mia_2d_datafield_cxx

#include <stdexcept>

#include <mia/2d/2DDatafield.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& _Size):
	_M_size(_Size), 
	_M_data(new data_array(_M_size.x * _M_size.y))
{
}

template <class T> 
T2DDatafield<T>::T2DDatafield():
	_M_size(C2DBounds(0,0)), 
	_M_data(new data_array(0))
{
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& size,const T *_data):
	_M_size(size), 
	_M_data(new data_array(_M_size.x * _M_size.y))
{
	if (_data)
		::std::copy(_data, _data + _M_data->size(), _M_data->begin()); 
	else
		::std::fill(_M_data->begin(), _M_data->end(), T()); 
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& size, const data_array& data):
	_M_size(size),
	_M_data(new data_array(data))
{
	assert(_M_data->size() == _M_size.x * _M_size.y); 
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const T2DDatafield<T>& org):
	_M_size(org._M_size), 
	_M_data(org._M_data)
{
}

template <class T> 
T2DDatafield<T>::~T2DDatafield()
{
}
	
template <class T> 
void T2DDatafield<T>::make_single_ref()
{
	if (!_M_data.unique())
		_M_data = data_pointer(new data_array(*_M_data)); 
}

template <class T> 
typename T2DDatafield<T>::size_type T2DDatafield<T>::size()const
{
	return _M_data->size(); 
}

template <class T> 
T T2DDatafield<T>::get_interpol_val_at(const C2DFVector& p) const
{
	size_t  x = (size_t )p.x;
	size_t  y = (size_t )p.y;
	float  xp = p.x - x; float  xm = 1.0 - xp;
	float  yp = p.y - y; float  ym = 1.0 - yp;
	
	const T& H00 = (*this)(x  ,y  );
	const T& H01 = (*this)(x  ,y+1);
	const T& H10 = (*this)(x+1,y  );
	const T& H11 = (*this)(x+1,y+1);
	
	return T(ym * ( xm * H00 + xp * H10) + 
		 yp * ( xm * H01 + xp * H11));

}

template <class T> 
const C2DBounds&  T2DDatafield<T>::get_size() const
{
	return _M_size;
}

template <class T> 
void T2DDatafield<T>::clear()
{
	make_single_ref();
	std::fill(begin(), end(),T()); 
}
	
template <class T> 	
typename T2DDatafield<T>::reference
T2DDatafield<T>::operator()(size_t  x, size_t  y)
{
	if (x < _M_size.x && y < _M_size.y){	
		return (*_M_data)[x + _M_size.x * y ];
	}else{
		//FORCECOREDUMP;
		throw "operator(x,y,z):Index out of bounds";
	}

}

template <class T> 
typename T2DDatafield<T>::const_reference
T2DDatafield<T>::operator()(size_t  x, size_t  y)const
{
	const data_array& cData = *_M_data; 
	
	if ( x < _M_size.x && y < _M_size.y ){
		return cData[ x + _M_size.x * y ];
	}else{
		return Zero;
	}
}
template <class T> 
typename T2DDatafield<T>::const_reference
T2DDatafield<T>::operator()(const C2DBounds& l)const
{
	const data_array& cData = *_M_data; 
	
	if ( l.x < _M_size.x && l.y < _M_size.y ){
		return cData[ l.x + _M_size.x * l.y ];
	}else{
		return Zero;
	}
}

template <class T> 
typename T2DDatafield<T>::reference
T2DDatafield<T>::operator()(const C2DBounds& l)
{
	return (*this)(l.x,l.y);
}

template <class T> 
void T2DDatafield<T>::get_data_line_x(size_t y, std::vector<T>& buffer)const
{
	assert(y < _M_size.y); 
	buffer.resize(_M_size.x); 

	const data_array& d = *_M_data; 
	const_iterator b = d.begin(); 
	advance(b, y * _M_size.x); 

	std::copy(b, b + _M_size.x, buffer.begin()); 
}
     
template <class T> 
void T2DDatafield<T>::get_data_line_y(size_t x, std::vector<T>& buffer)const
{
	assert(x < _M_size.x); 
	buffer.resize(_M_size.y); 

	const data_array& d = *_M_data; 
	const_iterator src = d.begin(); 
	advance(src,x);
	typename std::vector<T>::iterator dest_i = buffer.begin(); 
	typename std::vector<T>::iterator dest_e = buffer.end(); 
	
	size_t idx = 0; 
	while (dest_i != dest_e) {
		*dest_i = src[idx];
		idx += _M_size.x;
		++dest_i;
	}
}

template <class T> 
void T2DDatafield<T>::put_data_line_x(size_t y,  const std::vector<T>& buffer)
{
	assert(y < _M_size.y); 
	assert(buffer.size() == _M_size.x); 
	make_single_ref();

	std::copy(buffer.begin(), buffer.end(), _M_data->begin() + _M_size.x * y ); 
}
     
template <class T> 
void T2DDatafield<T>::put_data_line_y(size_t x, const std::vector<T>& buffer)
{
	assert(x < _M_size.x); 
	assert(buffer.size() == _M_size.y); 
	
	make_single_ref();
	typename std::vector<T>::const_iterator src_i = buffer.begin();
	typename std::vector<T>::const_iterator src_e = buffer.end();
	
	iterator dest = begin(); 
	advance(dest, x); 

	size_t idx = 0; 
	while ( src_i != src_e ) {
		dest[idx] = *src_i;
		idx += _M_size.x; 
		++src_i;
	}
}

template <class T> 
T2DDatafield<T>& T2DDatafield<T>::operator = (const T2DDatafield<T>& org)
	
{
	if (&org == this){
		return *this;
	}
	
	_M_size = org._M_size;
	_M_data = org._M_data;
	return *this; 
}



template <class T> 
const T T2DDatafield<T>::Zero = T();

NS_MIA_END

#endif




