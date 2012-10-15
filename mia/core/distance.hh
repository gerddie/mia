
/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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



#ifndef mia_core_distance_hh
#define mia_core_distance_hh

#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   This function evaluates the 1D distance transform 
   of an input function 
   \param[in,out] r at input the squared values of the function to evaluate the distance to 
   at output it contains the distances 
 */

void EXPORT_CORE distance_transform_inplace(std::vector<float>& r); 


template <typename InputIterator, typename OutputIterator, typename T> 
struct __distance_transform_prepare {
	static void apply(InputIterator in_begin, InputIterator in_end, OutputIterator out_begin){
		std::transform(in_begin, in_end, out_begin, [](float x){ return x*x;});
	}
}; 

template <typename InputIterator, typename OutputIterator> 
struct __distance_transform_prepare<InputIterator, OutputIterator, bool> {
	static void apply(InputIterator in_begin, InputIterator in_end, OutputIterator out_begin){
		std::transform(in_begin, in_end, out_begin, 
			       [](bool x){ return x ? 0.0f : std::numeric_limits<float>::max();});
	}
}; 

template <typename InputIterator, typename OutputIterator>
void  distance_transform_prepare(InputIterator in_begin, InputIterator in_end, 
					    OutputIterator out_begin) 
{
	typedef typename std::iterator_traits<InputIterator>::value_type in_value_type; 
	__distance_transform_prepare<InputIterator, OutputIterator, in_value_type>::apply( in_begin, in_end, out_begin); 
}

NS_MIA_END

#endif
