/* -*- mia-c++  -*-
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


#ifndef mia_core_traits__hh
#define mia_core_traits__hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   A template to get an generic dimension vector 
   \tparam T must be a template that defines a type \a dimsize_type
 */
template <template <typename> class T>
struct dim_traits {
	typedef typename T::dimsize_type dimsize_type; 
};

/**
   Structure to enable the use of the dim_traits class for std::vector 
 */
struct vector_dimsize {
	
	vector_dimsize(size_t n):_M_n(n) {
	}
	
	size_t size() {
		return 1; 
	}
	T& operator [] (int i)
	{
		return _M_n; 
	}
	const T& operator [] (int i) const 
	{
		return _M_n; 
	}
	size_t _M_n; 
}; 

template <typename T>
struct dim_traits<std::vector<T> > {
		
	typedef vector_dimsize dimsize_type; 
}; 



NS_MIA_END

#endif
