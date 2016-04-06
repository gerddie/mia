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

#include <mia/2d/shapes/rect.hh>

using namespace std; 
NS_MIA_USE; 


BOOST_AUTO_TEST_CASE( test_rect_filled ) 
{
	auto shape = BOOST_TEST_create_from_plugin<CRectangle2DShapePlugin>("rectangle:width=3,height=4,fill=1"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(7,9)); 
	auto mask = shape->get_mask(); 
	
	BOOST_CHECK_EQUAL(mask.get_size().x, 7u); 
	BOOST_CHECK_EQUAL(mask.get_size().y, 9u); 
	
	for (auto i = mask.begin(); i != mask.end(); ++i) {
		BOOST_CHECK(*i); 
	}
}

BOOST_AUTO_TEST_CASE( test_rect_not_filled ) 
{
	auto shape = BOOST_TEST_create_from_plugin<CRectangle2DShapePlugin>("rectangle:width=3,height=4,fill=0"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(7,9)); 
	auto mask = shape->get_mask(); 
	
	BOOST_CHECK_EQUAL(mask.get_size().x, 7u); 
	BOOST_CHECK_EQUAL(mask.get_size().y, 9u); 

	auto im = mask.begin(); 
	for (size_t y = 0; y < 9; ++y) 
		for (size_t x = 0; x < 7; ++x, ++im)
			BOOST_CHECK_EQUAL(*im, y == 0 || y == 8 || x == 0 || x == 6); 
	
}

BOOST_AUTO_TEST_CASE( test_square_filled ) 
{
	auto shape = BOOST_TEST_create_from_plugin<CSquare2DShapePlugin>("square:width=3,fill=1"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(7,7)); 
	auto mask = shape->get_mask(); 
	
	BOOST_CHECK_EQUAL(mask.get_size().x, 7u); 
	BOOST_CHECK_EQUAL(mask.get_size().y, 7u); 
	
	for (auto i = mask.begin(); i != mask.end(); ++i) {
		BOOST_CHECK(*i); 
	}
}
