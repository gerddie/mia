/* -*- mia-c++  -*-
 * Copyright (c) 2004 
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: 3DDatafield.hh 732 2005-11-14 18:06:39Z write1 $

/*! \brief A templated datafield class
 
The class holds all types of data stored in three dimensional fields.
 
\file 3DDatafield.cxx
\author Gert Wollny <wollny@cbs.mpg.de>
 
*/

#ifndef __3ddatafield_cxx
#define __3ddatafield_cxx

#include <mia/core/msgstream.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/3d/3DDatafield.hh>


NS_MIA_BEGIN





template <class T>
T3DDatafield<T>::T3DDatafield():
	_M_size(C3DBounds(0,0,0)), 
	_M_xy(0), 
	_M_data(new std::vector<T>(0))
{
}

template <class T>
void T3DDatafield<T>::swap(T3DDatafield& other)
{
	::std::swap(_M_size, other._M_size); 
	::std::swap(_M_xy, other._M_xy); 
	_M_data.swap(other._M_data); 
}

template <class T>
T3DDatafield<T>::T3DDatafield(const C3DBounds& size ):
	_M_size(size),
	_M_xy(size.x * size.y), 
	_M_data(new std::vector<T>(size.x * size.y * size.z))
{
}

template <class T>
T3DDatafield<T>::T3DDatafield(const C3DBounds& size, const T *data):
	_M_size(size), 
	_M_xy(size.x * size.y), 
	_M_data(new std::vector<T>(size.x * size.y * size.z))
{
	std::copy(data, data + size.x * size.y * size.z, _M_data->begin()); 
}
	
template <class T>
T3DDatafield<T>::~T3DDatafield()
{
}

template <class T>
void T3DDatafield<T>::make_single_ref()
{
	if (!_M_data.unique())
		_M_data = ref_data_type( new std::vector<T>(*_M_data) );
}

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_interpol_val_at(const T3DVector<float >& p) const
{
        return get_trilin_interpol_val_at(p);
}

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::operator()(const T3DVector<float >& pos)const
{
        return get_trilin_interpol_val_at(pos);
}

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_trilin_interpol_val_at(const T3DVector<float >& p) const
{
        // Calculate the coordinates and the distances
        size_t  x = (size_t )p.x;
        size_t  y = (size_t )p.y;
        size_t  z = (size_t )p.z;
        float  fx = p.x-x;
        float  fy = p.y-y;
        float  fz = p.z-z;

        float  dx = 1-fx;
        float  dy = 1-fy;
        float  dz = 1-fz;

        if ( x < _M_size.x-1 && y  < _M_size.y -1 && z < _M_size.z -1 ) {

                const T *ptr = &(*this)[x + _M_size.x * (y +_M_size.y * z)];
                const T *ptr_h = &ptr[_M_size.x];
                const T *ptr2 = &ptr[_M_xy];
                const T *ptr2_h = &ptr2[_M_size.x];
                const T a1 = T(dx * ptr[0]    + fx * ptr[1]);
                const T a3 = T(dx * ptr_h[0]  + fx * ptr_h[1]);
                const T a5 = T(dx * ptr2[0]   + fx * ptr2[1]);
                const T a7 = T(dx * ptr2_h[0] + fx * ptr2_h[1]);
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        } else {
                const T a1 = T(dx * (*this)(x  , y  , z  ) + fx * (*this)(x+1, y  , z  ));
                const T a3 = T(dx * (*this)(x  , y+1, z  ) + fx * (*this)(x+1, y+1, z  ));
                const T a5 = T(dx * (*this)(x  , y  , z+1) + fx * (*this)(x+1, y  , z+1));
                const T a7 = T(dx * (*this)(x  , y+1, z+1) + fx * (*this)(x+1, y+1, z+1));
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        }
}



template <class T>
void T3DDatafield<T>::get_data_line_x(int y, int z, std::vector<T>& result)const
{
        result.resize(_M_size.x);
	size_t start = _M_size.x * (y  + _M_size.y * z); 
	
	std::copy(_M_data->begin() + start, _M_data->begin() + start + _M_size.x, result.begin()); 
}

template <class T>
void T3DDatafield<T>::get_data_line_y(int x, int z, std::vector<T>& result)const
{
        result.resize(_M_size.y);
	size_t start = x + _M_size.x * _M_size.y * z;

	typename std::vector<T>::const_iterator i = _M_data->begin() + start; 
	
	for (typename std::vector<T>::iterator k = result.begin(); k != result.end(); ++k, i += _M_size.x)
                *k = *i;
}

template <class T>
void T3DDatafield<T>::get_data_line_z(int x, int y, std::vector<T>& result)const
{
        result.resize(_M_size.z);
	size_t start = x + _M_size.x * y;


	typename std::vector<T>::const_iterator i = _M_data->begin() + start;
	typename std::vector<T>::iterator k = result.begin(); 
	
	if (_M_size.z > 8) {
		const size_t xy = _M_xy; 
		const size_t xy2 = _M_xy << 1; 
		const size_t xy3 = _M_xy + xy2; 
		const size_t xy4 =  xy2 << 1; 
		int  z = _M_size.z - 4; 
		while ( z > 0) {
			k[0] = i[0];
			k[1] = i[xy];
			k[2] = i[xy2];
			k[3] = i[xy3];
			z -= 4; 
			k += 4;
			i += xy4;
		}
	}
		
	for (; k != result.end(); ++k, i += _M_xy)
		*k = *i;

}


template <class T>
void T3DDatafield<T>::put_data_line_x(int y, int z, const std::vector<T>& input)
{
        assert(input.size() == _M_size.x);
	
	make_single_ref();
		
        size_t start = _M_size.x * (y  + _M_size.y * z);
	
	std::copy(input.begin(), input.end(), _M_data->begin() + start); 
}

template <class T>
void T3DDatafield<T>::put_data_line_y(int x, int z, const std::vector<T>& input)
{
        assert(input.size() == _M_size.y);
	
	make_single_ref();
	size_t start= x + _M_size.x * _M_size.y * z;
	
	typename std::vector<T>::iterator k = _M_data->begin() + start; 

	for (typename std::vector<T>::const_iterator i = input.begin(); 
	     i != input.end(); ++i, k += _M_size.x)
		*k = *i; 
}

template <class T>
void T3DDatafield<T>::put_data_line_z(int x, int y, const std::vector<T>& input)
{
        assert(input.size() == _M_size.z);

	make_single_ref();
	size_t start= x + _M_size.x * y;
	
	typename std::vector<T>::iterator k = _M_data->begin() + start; 

	for (typename std::vector<T>::const_iterator i = input.begin(); 
	     i != input.end(); ++i, k += _M_xy)
		 *k = *i; 
}


template <class T>
T3DDatafield<T>& T3DDatafield<T>::operator = (const T3DDatafield<T>& org)
{
        if (&org == this) {
                return *this;
        }
	
        _M_size = org._M_size;
        _M_xy = org._M_xy;
        _M_data = org._M_data;

        return *this;
}

template <class T>
T3DDatafield<T>::T3DDatafield(const T3DDatafield<T>& org):
	_M_size(org._M_size),
	_M_xy(org._M_xy),
	_M_data(org._M_data)
{
}

template <class T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_xy(size_t  z)const
{
	assert(z < get_size().z); 
	T2DDatafield<T> result = T2DDatafield<T>(C2DBounds(get_size().x, get_size().y)); 
	copy(begin_at(0,0,z), begin_at(0,0,z) + result.size(), result.begin()); 
	return result; 
}

template <class T>
void T3DDatafield<T>::put_data_plane_xy(size_t  z, const T2DDatafield<T>& p)
{
	assert(z < get_size().z); 
	assert(p.get_size().x == get_size().x && p.get_size().y == get_size().y); 
	copy(p.begin(), p.end(), begin_at(0,0,z)); 
}

template <class T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_yz(size_t  x)const
{
	assert(x < get_size().x); 

	C2DBounds size(get_size().y, get_size().z); 
	const size_t dx = get_size().x; 

	T2DDatafield<T> result(size); 
	typename T2DDatafield<T>::iterator r = result.begin(); 
	typename T3DDatafield<T>::const_iterator i = begin_at(x,0,0); 

	for (size_t z = 0; z < size.y; ++z) 
		for (size_t y = 0; y < size.x; ++y, ++r, i += dx) {
			*r = *i; 
		}
	return result; 
}

template <class T>
void T3DDatafield<T>::put_data_plane_yz(size_t  x, const T2DDatafield<T>& p)
{
	assert(p.get_size().x == get_size().y && p.get_size().y == get_size().z); 
	assert(x < get_size().x); 

	const C2DBounds& size = p.get_size(); 
	const size_t dx = get_size().x; 

	typename T2DDatafield<T>::const_iterator i = p.begin(); 
	typename T3DDatafield<T>::iterator r = begin_at(x,0,0); 

	for (size_t z = 0; z < size.y; ++z) 
		for (size_t y = 0; y < size.x; ++y, ++i, r += dx) {
			*r = *i; 
		}

}

template <class T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_xz(size_t  y)const
{
	assert(y < get_size().y); 
	
	C2DBounds size(get_size().x, get_size().z); 
	const size_t dxy = get_size().y * size.x; 

	T2DDatafield<T> result(size); 
	typename T2DDatafield<T>::iterator r = result.begin(); 
	typename T3DDatafield<T>::const_iterator i = begin_at(0, y, 0); 

	for (size_t z = 0; z < size.y; ++z, i += dxy, r += size.x) {
		copy(i, i + size.x, r);  
	}	
	return result; 
}

template <class T>
void  T3DDatafield<T>::put_data_plane_xz(size_t  y, const T2DDatafield<T>& p)
{
	assert(y < get_size().y); 
	assert(p.get_size().x == get_size().x && p.get_size().y == get_size().z); 

	const size_t dxy = get_size().y * get_size().x; 

	const C2DBounds& size = p.get_size(); 

	T2DDatafield<T> result(size); 
	typename T3DDatafield<T>::iterator r = begin_at(0, y, 0); 
	typename T2DDatafield<T>::const_iterator i = p.begin(); 

	for (size_t z = 0; z < size.y; ++z, i += size.x, r += dxy) {
		copy(i, i + size.x, r);  
	}
}

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_avg()
{
        T Avg = T();
        // first calculate avrg.
        const_iterator i = begin();
        const_iterator e = end();
        while ( i != e ) {
                Avg += *i++;
        }
        Avg /= (_M_size.x * _M_size.y *_M_size.z);

        return Avg;
}

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::strip_avg()
{
        make_single_ref();
        T Avg = get_avg();
        // first calculate avrg.
        iterator r = begin();
        iterator e = end();

        while ( r != e ) {
                *r++ -= Avg;
        }
        return Avg;
}

template <class T>
void T3DDatafield<T>::clear()
{
        make_single_ref();
	std::fill(_M_data->begin(), _M_data->end(), T()); 
}

template <class T>
const T T3DDatafield<T>::Zero = T();

template <class T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_block_avrg(const C3DBounds& Start, const C3DBounds& BlockSize) const
{
        T Result = 0; // Not the best approach
        size_t  S = BlockSize.x * BlockSize.y * BlockSize.z;
        C3DBounds End = Start+BlockSize;
        for (size_t  z = Start.z; z < End.z; z++) {
                for (size_t  y = Start.y; y < End.y; y++) {
                        for (size_t  x = Start.x; x < End.x; x++) {
                                Result += (*this)(x,y,z);
                        }
                }
        }
        return Result / S;
}

NS_MIA_END

#endif
