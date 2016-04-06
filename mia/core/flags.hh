/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_core_flags_hh
#define mia_core_flags_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN 

/**
   This macro implements operations on a strongly typed enum 
   that is used as a flag. We use a macro since we don't want to 
   provide a template that would be valid for all types that don't 
   have these operators implemented 
*/

#define IMPLEMENT_FLAG_OPERATIONS(E)					\
	inline E operator | (E lhs, E rhs)				\
	{								\
		return static_cast<E>(  static_cast<int>(lhs) | static_cast<int>(rhs)); \
	}								\
									\
	inline E operator  & (E lhs, E rhs)				\
	{								\
		return static_cast<E>(  static_cast<int>(lhs) & static_cast<int>(rhs)); \
	}								\
									\
	inline E operator -= (E& lhs, E rhs)				\
	{								\
		lhs = static_cast<E>(  static_cast<int>(lhs) & ~static_cast<int>(rhs)); \
		return lhs;						\
	}								\
									\
	inline bool has_flag(E flags, E test)				\
	{								\
		return (flags & test) == test;				\
	}


NS_MIA_END 
#endif 
