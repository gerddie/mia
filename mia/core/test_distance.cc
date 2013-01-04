/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/internal/autotest.hh>
#include <mia/core/distance.hh>

NS_MIA_USE; 
using std::vector; 
using std::numeric_limits; 

BOOST_AUTO_TEST_CASE( test_distance_from_inifinity ) 
{
	vector<bool> in_1d { 0, 0, 0, 1, 
			0, 0, 0, 0, 
			0, 0, 1, 0, 
			0, 0, 0, 0 }; 
	float out_1d[16] = { 9, 4, 1, 0, 
			     1, 4, 9, 9, 
			     4, 1, 0, 1, 
			     4, 9, 16, 25 };
	
	vector<float> src(16); 
	distance_transform_prepare(in_1d.begin(), in_1d.end(),src.begin()); 	
	distance_transform_inplace(src);
	
	for (size_t i = 0; i < 16; ++i) {
		BOOST_CHECK_CLOSE(src[i], out_1d[i], 0.1); 
	}
}

BOOST_AUTO_TEST_CASE( test_distance_from_function ) 
{
	float in_1d[16]  = { 20, 2, 11, 0, 7, 9, 12, 12, 17, 100, 0,  2, 100,  4, 100, 100}; 

	float out_1d[16] = {  5, 4,  1, 0, 1, 4,  9,  9,  4,   1, 0,  1,   4,  8,  13,  20};
	
	vector<float> src(16); 
	distance_transform_prepare(&in_1d[0], &in_1d[16],src.begin()); 
	distance_transform_inplace(src);
	
	for (size_t i = 0; i < 16; ++i) {
		BOOST_CHECK_CLOSE(src[i], out_1d[i], 0.1); 
	}
}
