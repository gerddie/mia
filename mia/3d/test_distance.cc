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
#include <mia/3d/distance.hh>

BOOST_AUTO_TEST_CASE( test_distance_full3d_inf ) 
{
	float src[64] = {
		0, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1, 
		1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 0, 1,  1, 1, 1, 1, 
		1, 1, 1, 1,   1, 0, 1, 1,   1, 1, 1, 1,  1, 1, 1, 1
		1, 1, 1, 1,   1, 1, 1, 1,   1, 0, 1, 1,  1, 1, 1, 1
	}; 
	
	float test[64] = {
		0, 1, 4, 9,   
		1, 2, 2,10,  
		4, 2, 1, 2,  
		9,10, 2,18, 

		1, 2, 1, 1,   
		2, 1, 1, 2,   
		3, 1, 0, 1,  
		6, 2, 1, 2, 
		
		2, 1, 2, 6,   
		1, 0, 1, 3,   
		2, 1, 1, 2,  
		5, 4, 2, 3, 

		5, 4, 5, 8,   
		2, 1, 2, 5,   
		1, 0, 1, 4,  
		2, 1, 2, 5
	}
	
	C3DFImage src_img(C3DBounds(4,4,4)); 
	
	transform(&src[0], &src[16], src_img.begin(), [](float x){return numeric_limits<float>::max() * x;}); 
	
	C3DFImage result =  distance_transform(src_img); 

	int k = 0; 
	for(auto i = result.begin(); i != result.end(); ++i, ++k ) {
		BOOST_CHECK_CLOSE(*i, test[k], 0.1); 
		cvdebug() << "k=" << k << ", " << *i << ", " << test[k] << "\n"; 
	}

}
