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
#include <mia/2d/morphshape.hh>
#include <mia/2d/imagetest.hh>

NS_MIA_USE; 

BOOST_AUTO_TEST_CASE( test_simple_foreground_shape )
{
	C2DMorphShape shape; 
	C2DMorphShape::value_type pixel(1,0); 

	shape.add_pixel(pixel, true);
	
	BOOST_CHECK_EQUAL(shape.get_foreground_mask().size(), 1u); 
	BOOST_CHECK_EQUAL(shape.get_background_mask().size(), 0u); 

	BOOST_CHECK_EQUAL(*shape.get_foreground_mask().begin(), pixel); 
	
}

BOOST_AUTO_TEST_CASE( test_simple_background_shape )
{
	C2DMorphShape shape; 
	C2DMorphShape::value_type pixel(1,0); 

	shape.add_pixel(pixel, false);
	
	BOOST_CHECK_EQUAL(shape.get_foreground_mask().size(), 0u); 
	BOOST_CHECK_EQUAL(shape.get_background_mask().size(), 1u); 

	BOOST_CHECK_EQUAL(*shape.get_background_mask().begin(), pixel); 
	
}

BOOST_AUTO_TEST_CASE( test_mask_init_constructor_error )
{
	P2DShape shape(new C2DShape()); 
	shape->insert(C2DShape::value_type(1,1)); 
	
	BOOST_CHECK_THROW(C2DMorphShape(shape, shape), invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_mask_init_add_pixel_errors )
{
	C2DMorphShape shape; 
	C2DMorphShape::value_type pixel1(1,1); 
	C2DMorphShape::value_type pixel2(-1,1); 
	shape.add_pixel(pixel1, false); 
	shape.add_pixel(pixel2, true); 
	
	BOOST_CHECK_THROW(shape.add_pixel(pixel1, true), invalid_argument); 
	BOOST_CHECK_THROW(shape.add_pixel(pixel2, false), invalid_argument);

}

BOOST_AUTO_TEST_CASE( test_mask_check_rotate )
{
	C2DMorphShape shape; 
	C2DMorphShape::value_type pixel1(1,0); 
	C2DMorphShape::value_type pixel2(-1,1); 
	C2DMorphShape::value_type pixel3( 0,1); 

	C2DMorphShape::value_type pixel1r90(0, -1); 
	C2DMorphShape::value_type pixel2r90(1,1); 
	C2DMorphShape::value_type pixel3r90(1,0); 

	shape.add_pixel(pixel1, false); 
	shape.add_pixel(pixel2, true); 
	shape.add_pixel(pixel3, true);
	
	auto rshape = shape.rotate_by_90(); 

	auto fg  = rshape.get_foreground_mask(); 

	BOOST_CHECK_EQUAL(fg.size(), 2u); 
	BOOST_CHECK(fg.has_location(pixel2r90)); 
	BOOST_CHECK(fg.has_location(pixel3r90)); 


	auto bg  = rshape.get_background_mask(); 
	
	BOOST_CHECK_EQUAL(bg.size(), 1u); 	
	BOOST_CHECK(bg.has_location(pixel1r90)); 

}



BOOST_AUTO_TEST_CASE( test_simple_hit_and_miss ) 
{
	const C2DBounds size(9, 7); 
	
	vector<bool> input_image = {
		0, 1, 1, 0, 1, 0, 1, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 1, 0, 0,
		0, 1, 0, 1, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 1, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0
	}; 

	vector<bool> test_image = {
		0, 0, 0, 0, 1, 0, 1, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0
	}; 
	
	// the "find singular pixel" shape
	C2DMorphShape shape; 
	shape.add_pixel(C2DMorphShape::value_type(0,0), true); 

	shape.add_pixel(C2DMorphShape::value_type(-1,-1), false); 
	shape.add_pixel(C2DMorphShape::value_type(-1, 0), false); 
	shape.add_pixel(C2DMorphShape::value_type(-1, 1), false); 
	shape.add_pixel(C2DMorphShape::value_type( 0,-1), false); 
	shape.add_pixel(C2DMorphShape::value_type( 0, 1), false); 
	shape.add_pixel(C2DMorphShape::value_type( 1,-1), false); 
	shape.add_pixel(C2DMorphShape::value_type( 1, 0), false); 
	shape.add_pixel(C2DMorphShape::value_type( 1, 1), false); 

	
	C2DBitImage source(size, input_image); 
	C2DBitImage target(size); 
	C2DBitImage expect(size, test_image); 

	BOOST_CHECK_EQUAL(morph_hit_and_miss_2d(target, source, shape), 7u);
	
	test_image_equal(target, expect);


}


