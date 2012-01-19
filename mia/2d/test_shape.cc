/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <mia/2d/shape.hh>
#include <mia/core/test_helpers.hh>
#include <mia/2d/imagetest.hh>

NS_MIA_USE
using namespace std; 
using namespace boost;

C2DShapePluginHandlerTestPath test_path; 


BOOST_AUTO_TEST_CASE( test_2dshape_handler ) 
{
	set<string> test_data = { 
		"1n", "4n", "8n", "sphere","rectangle", "square"
	}; 
	test_plugin_names_and_count<C2DShapePluginHandler>(test_data); 
}



BOOST_AUTO_TEST_CASE( test_shape_rotate_rotation_symetrics ) 
{
	set<string> test_data = { 
		"1n", "4n", "8n", "sphere:r=3","square"
	};

	for (auto s = test_data.begin(); s != test_data.end(); ++s) {
		auto shape = produce_2d_shape(*s);
		P2DShape rshape = rotate_90_degree(*shape);
		test_image_equal(rshape->get_mask(), shape->get_mask());
	}
	
}

BOOST_AUTO_TEST_CASE( test_shape_rotate_nonsymetric ) 
{
	auto initial_shape = produce_2d_shape("rectangle:height=2,width=4"); 
	auto test_shape = produce_2d_shape("rectangle:height=4,width=2");

	P2DShape rshape = rotate_90_degree(*initial_shape);
	
	test_image_equal(rshape->get_mask(), test_shape->get_mask());
	
}
