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

#include <mia/internal/autotest.hh>
#include <mia/core/distance.hh>
#include <mia/3d/distance.hh>

using namespace mia; 


BOOST_AUTO_TEST_CASE( test_distance_full3d_inf ) 
{
	bool src[64] = {
		1, 0, 0, 0,   
		0, 0, 0, 0,   
		0, 0, 0, 0,  
		0, 0, 0, 0, 
		
		0, 0, 0, 0,   
		0, 0, 0, 0,   
		0, 0, 1, 0,  
		0, 0, 0, 0, 

		0, 0, 0, 0,   
		0, 1, 0, 0,   
		0, 0, 0, 0,  
		0, 0, 0, 0,
		
		0, 0, 0, 0,   
		0, 0, 0, 0,   
		0, 1, 0, 0,  
		0, 0, 0, 0
	}; 
	
	float test[64] = {
		0, 1, 4, 6,   
		1, 2, 2, 3,
		4, 2, 1, 2,  
		6, 3, 2, 3, 

		1, 2, 3, 5,   
		2, 1, 1, 2,   
		3, 1, 0, 1,  
		5, 2, 1, 2, 
		
		2, 1, 2, 5,   
		1, 0, 1, 3,   
		2, 1, 1, 2,  
		3, 2, 2, 3, 

		3, 2, 3, 6,   
		2, 1, 2, 5,   
		1, 0, 1, 4,  
		2, 1, 2, 5
	}; 
	
	C3DFImage src_img(C3DBounds(4,4,4)); 
	
	distance_transform_prepare(&src[0], &src[64], src_img.begin()); 
	
	C3DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		BOOST_CHECK_CLOSE(*i, test[k], 0.1); 
	}
}


BOOST_AUTO_TEST_CASE( test_distance_full3d_func ) 
{
	float src[64] = {
		1, 2, 4, 5,   
		5, 4, 3, 2,   
		3, 2, 2, 3,  
		4, 5, 3, 1, 
		
		1, 4, 5, 2,   
		1, 2, 3, 3,   
		2, 3, 1, 4,  
		4, 4, 3, 4, 

		3, 2, 1, 1,   
		2, 1, 4, 3,   
		0, 3, 2, 1,  
		5, 4, 4, 2,
		
		7, 8, 1, 2,   
		6, 5, 4, 3,   
		4, 1, 4, 2,  
		5, 5, 4, 3
	}; 
	
	float test[64] = {
		1, 2, 5, 5,   
		2, 3, 3, 4,   
		3, 3, 2, 2,  
	        5, 4, 2, 1, 
		
		1, 2, 2, 2,   
		1, 2, 2, 3,   
		1, 2, 1, 2,  
		2, 3, 2, 2, 

		2, 2, 1, 1,   
		1, 1, 2, 2,   
		0, 1, 2, 1,  
		1, 2, 3, 2,
		
		4, 2, 1, 2,   
		2, 2, 2, 3,   
		1, 1, 2, 2,  
		2, 2, 3, 3
	}; 
	
	C3DFImage src_img(C3DBounds(4,4,4)); 
	
	distance_transform_prepare(&src[0], &src[64], src_img.begin()); 
	
	C3DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		cvdebug() << "k=" << k << ":" << *i << ", vs " << test[k] << "\n"; 
		BOOST_CHECK_CLOSE(*i, test[k], 0.1); 
	}
}
