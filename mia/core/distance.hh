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

#ifndef mia_core_distance_hh
#define mia_core_distance_hh

#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   This function evaluates the 1D distance transform of an input function. 
   If the input function is given as raw data that is not the result of another 
   call to this distance transform function, then one should call 
   distance_transform_prepare to properly prepare the data for the distance transform. 
   \param[in,out] r at input the squared values of the function to evaluate the distance to 
   at output it contains the squared distances.
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


/**
   This function evaluates prepares data for the use in a distance transform. 
   The input values are interpreted differently depending on the input data type: 
   If the values are given as boolean values then it is assumed that they belong to a mask, and 
   at each location x where f(x)==true, the function is assumed to be zero, and at each 
   f(x)==false the function is assumed to be inf+ (given as numeric_limits<float>::max()). 
   Otherwiese the values are interpreted directly, and the preparation for the dist-transform 
   consists in evaluating the square of each value 
   \tparam InputIterator input data iterator 
   \tparam OutputIterator output data iterator 
   \param in_begin start of the input data range 
   \param in_end end of the input data range 
   \param out_begin begin of the output data container, note that the output container must at least hold as 
   many values as the input data range provides. This is not tested. 
*/

template <typename InputIterator, typename OutputIterator>
void  distance_transform_prepare(InputIterator in_begin, InputIterator in_end, 
					    OutputIterator out_begin) 
{
	typedef typename std::iterator_traits<InputIterator>::value_type in_value_type; 
	__distance_transform_prepare<InputIterator, OutputIterator, in_value_type>::apply( in_begin, in_end, out_begin); 
}

NS_MIA_END

#endif
