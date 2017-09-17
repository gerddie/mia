/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#undef mia_2d_datafield_cxx
#ifndef mia_2d_datafield_cxx
#define mia_2d_datafield_cxx

#include <stdexcept>

#include <mia/2d/datafield.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

template <class T>  T2DDatafield<T>::T2DDatafield():
	m_size(0,0)
{
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const T2DDatafield<T>& org):
	m_size(org.m_size), 
	m_data(org.m_data)
{
}


template <class T> 
T2DDatafield<T>& T2DDatafield<T>::operator = (const T2DDatafield<T>& org)
{
	if (this != &org) {
		m_size = org.m_size; 
		m_data = org.m_data;
	}
	return *this; 
}


template <class T> 
T2DDatafield<T>::T2DDatafield(T2DDatafield<T>&& org):
	m_size(org.m_size), 
	m_data(std::move(org.m_data))
{
}

template <class T> 
T2DDatafield<T>& T2DDatafield<T>::operator = (T2DDatafield<T>&& org)
{
	if (this != &org) {
		m_size = org.m_size;
		m_data = std::move(org.m_data);
		
	}
	return *this; 
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& _Size):
	m_size(_Size), 
	m_data(m_size.x * m_size.y)
{
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& size,const T *_data):
	m_size(size), 
	m_data(m_size.x * m_size.y)
{
	if (_data)
		::std::copy(_data, _data + m_data.size(), m_data.begin()); 
	else
		::std::fill(m_data.begin(), m_data.end(), T()); 
}

template <class T> 
T2DDatafield<T>::T2DDatafield(const C2DBounds& size, const std::vector<T>& data):
	m_size(size),
	m_data(data.begin(), data.end())
{
	assert(m_data.size() == m_size.x * m_size.y); 
}

template <class T> 
T2DDatafield<T>::~T2DDatafield()
{
}
	
template <class T> 
void T2DDatafield<T>::make_single_ref()
{
 
}

template <class T> 
typename T2DDatafield<T>::size_type T2DDatafield<T>::size() const
{
	return m_data.size(); 
}

template <class T> 
const C2DBounds&  T2DDatafield<T>::get_size() const
{
	return m_size;
}

template <class T> 
void T2DDatafield<T>::clear()
{
	std::fill(begin(), end(),T()); 
}
	
template <class T> 	
typename T2DDatafield<T>::reference
T2DDatafield<T>::operator()(size_t  x, size_t  y)
{
	if (x < m_size.x && y < m_size.y){	
		return m_data[x + m_size.x * y ];
	}else{
		//FORCECOREDUMP;
		throw std::invalid_argument("T2DDatafield<T>::operator(x,y,z):Index out of bounds");
	}

}

template <class T> 
typename T2DDatafield<T>::const_reference
T2DDatafield<T>::operator()(size_t  x, size_t  y)const
{
	if ( x < m_size.x && y < m_size.y ){
		return m_data[ x + m_size.x * y ];
	}else{
		return Zero;
	}
}
template <class T> 
typename T2DDatafield<T>::const_reference
T2DDatafield<T>::operator()(const C2DBounds& l)const
{
	if ( l.x < m_size.x && l.y < m_size.y ){
		return m_data[ l.x + m_size.x * l.y ];
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
	assert(y < m_size.y); 
	buffer.resize(m_size.x); 
 
	const_iterator b = m_data.begin(); 
	advance(b, y * m_size.x); 

	std::copy(b, b + m_size.x, buffer.begin()); 
}
     
template <class T> 
void T2DDatafield<T>::get_data_line_y(size_t x, std::vector<T>& buffer)const
{
	assert(x < m_size.x); 
	buffer.resize(m_size.y); 
 
	const_iterator src = m_data.begin(); 
	advance(src,x);
	typename std::vector<T>::iterator dest_i = buffer.begin(); 
	typename std::vector<T>::iterator dest_e = buffer.end(); 
	
	size_t idx = 0; 
	while (dest_i != dest_e) {
		*dest_i = src[idx];
		idx += m_size.x;
		++dest_i;
	}
}

template <class T> 
void T2DDatafield<T>::put_data_line_x(size_t y,  const std::vector<T>& buffer)
{
	assert(y < m_size.y); 
	assert(buffer.size() == m_size.x); 

	std::copy(buffer.begin(), buffer.end(), m_data.begin() + m_size.x * y ); 
}
     
template <class T> 
void T2DDatafield<T>::put_data_line_y(size_t x, const std::vector<T>& buffer)
{
	assert(x < m_size.x); 
	assert(buffer.size() == m_size.y); 
	
	auto src_i = buffer.begin();
	auto src_e = buffer.end();
	
	iterator dest = begin(); 
	advance(dest, x); 

	size_t idx = 0; 
	while ( src_i != src_e ) {
		dest[idx] = *src_i;
		idx += m_size.x; 
		++src_i;
	}
}

template <typename T>
typename T2DDatafield<T>::Range
T2DDatafield<T>::get_range(const C2DBounds& start, const C2DBounds& end)
{
	return Range(start, end, *this); 
}

template <typename T>
typename T2DDatafield<T>::ConstRange
T2DDatafield<T>::get_range(const C2DBounds& start, const C2DBounds& end) const
{
	return ConstRange(start, end, *this); 
}


template <typename T>
typename T2DDatafield<T>::range_iterator 
T2DDatafield<T>::begin_range(const C2DBounds& begin, const C2DBounds& end)
{
	return range_iterator(begin, get_size(), begin, end, 
			      begin_at(begin.x, begin.y)); 
}

template <typename T>
typename T2DDatafield<T>::range_iterator 
T2DDatafield<T>::end_range(const C2DBounds& begin, const C2DBounds& end)
{
	return range_iterator(end, get_size(), begin, end, 
			      begin_at(end.x, end.y)); 
}

template <typename T>
typename T2DDatafield<T>::const_range_iterator 
T2DDatafield<T>::begin_range(const C2DBounds& begin, const C2DBounds& end)const
{
	return const_range_iterator(begin, get_size(), begin, end, 
				    begin_at(begin.x, begin.y)); 
}

template <typename T>
typename T2DDatafield<T>::const_range_iterator 
T2DDatafield<T>::end_range(const C2DBounds& begin, const C2DBounds& end)const
{
	return const_range_iterator(end, get_size(), begin, end, 
				    begin_at(end.x, end.y)); 
}

template <typename T>
typename T2DDatafield<T>::Range::iterator T2DDatafield<T>::Range::begin()
{
	return m_begin; 
}

template <typename T>
typename T2DDatafield<T>::Range::iterator T2DDatafield<T>::Range::end()
{
	return m_end; 
}

template <typename T>
T2DDatafield<T>::Range::Range(const C2DBounds& start, const C2DBounds& end, T2DDatafield<T>& field):
	m_begin(field.begin_range(start, end)), m_end(field.end_range(start, end))
{
}
					    

template <typename T>
typename T2DDatafield<T>::ConstRange::iterator T2DDatafield<T>::ConstRange::begin() const 
{
	return m_begin; 
}

template <typename T>
typename T2DDatafield<T>::ConstRange::iterator T2DDatafield<T>::ConstRange::end() const 
{
	return m_end; 
}


template <typename T>
T2DDatafield<T>::ConstRange::ConstRange(const C2DBounds& start, const C2DBounds& end, const T2DDatafield<T>& field):
	m_begin(field.begin_range(start, end)), m_end(field.end_range(start, end))
{
}

template <typename T>
T2DDatafield<T>::ConstRange::ConstRange(const Range& range):
	m_begin(range.m_begin), m_end(range.m_end)
{
}


template <class T> 
const typename  T2DDatafield<T>::value_type T2DDatafield<T>::Zero = T();

NS_MIA_END

#endif




