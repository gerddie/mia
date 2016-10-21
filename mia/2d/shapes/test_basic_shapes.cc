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

#include <mia/2d/shapes/basic_shapes.hh>

using namespace std; 
using namespace basic_2dshape_creator; 
NS_MIA_USE; 



BOOST_AUTO_TEST_CASE( test_1n ) 
{
	auto shape = BOOST_TEST_create_from_plugin<C1n2DShapeFactory>("1n"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(1,1)); 
	auto mask = shape->get_mask(); 

	BOOST_CHECK(mask(0,0)); 
}; 

BOOST_AUTO_TEST_CASE( test_4n ) 
{
	auto shape = BOOST_TEST_create_from_plugin<C4n2DShapeFactory>("4n"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(3,3)); 
	auto mask = shape->get_mask(); 

	BOOST_CHECK_EQUAL(mask(0,0), false); 
	BOOST_CHECK_EQUAL(mask(0,1), true ); 
	BOOST_CHECK_EQUAL(mask(0,2), false); 
	BOOST_CHECK_EQUAL(mask(1,0), true ); 
	BOOST_CHECK_EQUAL(mask(1,1), true ); 
	BOOST_CHECK_EQUAL(mask(1,2), true ); 
	BOOST_CHECK_EQUAL(mask(2,0), false); 
	BOOST_CHECK_EQUAL(mask(2,1), true ); 
	BOOST_CHECK_EQUAL(mask(2,2), false); 

}; 

BOOST_AUTO_TEST_CASE( test_8n ) 
{
	auto shape = BOOST_TEST_create_from_plugin<C8n2DShapeFactory>("8n"); 
	BOOST_REQUIRE(shape); 

	BOOST_CHECK_EQUAL(shape->get_size(), C2DBounds(3,3)); 
	auto mask = shape->get_mask(); 

	BOOST_CHECK_EQUAL(mask(0,0), true ); 
	BOOST_CHECK_EQUAL(mask(0,1), true ); 
	BOOST_CHECK_EQUAL(mask(0,2), true ); 
	BOOST_CHECK_EQUAL(mask(1,0), true ); 
	BOOST_CHECK_EQUAL(mask(1,1), true ); 
	BOOST_CHECK_EQUAL(mask(1,2), true ); 
	BOOST_CHECK_EQUAL(mask(2,0), true ); 
	BOOST_CHECK_EQUAL(mask(2,1), true ); 
	BOOST_CHECK_EQUAL(mask(2,2), true ); 
}; 

