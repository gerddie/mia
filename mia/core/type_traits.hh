/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#ifndef mia_core_typetraits_hh
#define mia_core_typetraits_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

template <typename T> 
struct plugin_data_type {
	typedef T type; 
}; 

template <typename T> 
struct atomic_data {
	typedef T type; 
	static const int size; 
}; 

template <typename T> 
const int atomic_data<T>::size = 1; 

/**
   This struct is a place holder for some templates. The actual template specialization 
   must implement the operator 
   bool operator()(T a, T b) const; 
   that returns true of lhs is smaller then rhs and false otherwise. The operator should implement 
   a strict weak ordering. 
   \tparam T the type to compare  
 */

template <typename T>
struct less_then {
	typedef T value_type; 
};


NS_MIA_END

#endif
