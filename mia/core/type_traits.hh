/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_typetraits_hh
#define mia_core_typetraits_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/// @cond INTERNAL 

template <typename T> 
struct __holder_type_dispatch  {
	typedef T type; 
}; 


template <> 
struct __holder_type_dispatch<bool>  {
	typedef unsigned char type; 
}; 


template <typename T>
struct __is_mask_pixel {
	static const bool value = false;
}; 

template <>
struct __is_mask_pixel<bool> {
	static const bool value = true;
}; 

/**
   @ingroup traits 
   @brief A trait to derive the actual type handled by a plug-in based on some 
   possibly unrelated type 
*/
template <typename T> 
struct plugin_data_type {
	typedef T type; 
}; 

/**
   @ingroup traits 
   @brief This trait provides information about the number of (equal typed) elements a type holds 
   and the element type. 
   @tparam T the type to provide information about 
   
   In the standard version, the element type is the same as the template parameter 
   and the number of elements is one. 
*/
template <typename T> 
struct atomic_data {
	typedef T type; 
	static const int size; 
}; 

template <typename T> 
const int atomic_data<T>::size = 1; 

/**
   \ingroup misc
   This struct is a place holder for some templates. The actual template specialization 
   must implement the operator 
   bool operator()(T a, T b) const; 
   that returns true of lhs is smaller then rhs and false otherwise. The operator should implement 
   a strict weak ordering. 
   \tparam T the type to compare  
 */

template <typename T>
struct less_then {
	/// type of the values handled by the operator 
	typedef T value_type; 
};

/// @endcond 

NS_MIA_END

#endif
