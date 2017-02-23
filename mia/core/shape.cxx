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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boost/static_assert.hpp>

#include <string>
#include <mia/core/shape.hh>

NS_MIA_BEGIN

template <typename T>
struct __adjust {
	static void apply(T& p, const T& q) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
	}
}; 

template <template <typename> class  T, typename M>
TShape<T,M>::TShape():
	m_shape(less_then<T<int> >())
{
	TRACE("TShape<T,M>::TShape()"); 
}

template <template <typename> class  T, typename M>
typename TShape<T,M>::const_iterator TShape<T,M>::begin() const
{
	return m_shape.begin(); 
}

template <template <typename> class  T, typename M>
typename TShape<T,M>::const_iterator TShape<T,M>::end() const
{
	return m_shape.end(); 
}

template <template <typename> class  T, typename M>
typename TShape<T,M>::Size TShape<T,M>::get_size() const
{
	return typename TShape<T,M>::Size(m_size); 
}

template <template <typename> class  T, typename M>
void TShape<T,M>::insert(const T<int>& p)
{
	m_shape.insert(p); 
	__adjust<T<int> >::apply(m_size,p);
}

template <template <typename> class  T, typename M>
bool TShape<T,M>::has_location(const T<int>& p) const
{
	return (m_shape.find(p) != m_shape.end()); 
}

template <template <typename> class  T, typename M>
typename TShape<T,M>::Mask TShape<T,M>::get_mask()const
{
	typename TShape<T,M>::Mask result(get_size());
	fill(result.begin(), result.end(), 0);  
	
	T<int> half_size = m_size / 2; 
		
	for (const_iterator i = begin(), e = end(); i != e; ++i){
		result(T<unsigned int>(*i + half_size)) = true;  
	}
	return result; 
}

template <template <typename> class  T, typename M>
typename TShape<T,M>::Flat::size_type TShape<T,M>::size() const
{
	return m_shape.size(); 
}

NS_MIA_END
