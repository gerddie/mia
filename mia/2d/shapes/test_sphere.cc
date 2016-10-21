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

#include <mia/internal/plugintester.hh>

#include <mia/2d/shapes/sphere.hh>

using namespace std; 
NS_MIA_USE; 


BOOST_AUTO_TEST_CASE( test_sphere ) 
{
	auto shape = BOOST_TEST_create_from_plugin<CSphere2DShapeFactory>("sphere:r=3"); 
	BOOST_REQUIRE(shape); 

	size_t s = 2 * 3 + 1;
	size_t r2 = 3*3;


	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(s,s)); 
	auto mask = shape->get_mask(); 
	
	
	BOOST_CHECK_EQUAL(mask.get_size().x, mask.get_size().y); 
	BOOST_CHECK_EQUAL(mask.get_size(), C2DBounds(s,s)); 
	
	for ( size_t y = 0; y < s; ++y)
		for ( size_t x = 0; x < s; ++x) {
			float r_h = (x - 3) * (x - 3) + (y - 3) * (y - 3);
			if (r_h <= r2) 
				BOOST_CHECK( mask(x,y)); 
			else 
				BOOST_CHECK(!mask(x,y)); 
		}
}; 
