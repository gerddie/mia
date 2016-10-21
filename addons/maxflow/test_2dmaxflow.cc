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
#include <mia/2d/imageio.hh>

#include <addons/maxflow/2dmaxflow.hh>

using namespace std;
using namespace mia; 
using maxflow_2dimage_filter::C2DMaxflowFilterPluginFactory; 

BOOST_AUTO_TEST_CASE ( test_2d_simple_maxflow )
{
// one source, one sink and a pretty simple 4x4 image

	// sink and source


	vector<unsigned char> image = {
		255, 255, 245, 233,
		200, 170, 170, 150,
		20,   30,  40,  30,
		40,   20,  50,  10
	};

	vector<bool> expect = {
		false, false, false, false,
		false, false, false, false,
		true, true, true, true, 
		true, true, true, true
	};
	
	C2DBounds sink(0,0);
	C2DBounds source(3,3);

	C2DBounds size(4,4);

	C2DUBImage in_image(size, image);

	// define a flow image
	// max flow = 1
	// min flow = 0

	C2DFImage *fimage = new C2DFImage(size);
	(*fimage)(sink) = 16.0f;  
	P2DImage sink_image(fimage);

	fimage = new C2DFImage(size);
	(*fimage)(source) = 16.0f; 
	P2DImage source_image(fimage);

       	auto maxflow = BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow:source-flow=src.@,sink-flow=sink.@");

	save_image("src.@", source_image);
	save_image("sink.@", sink_image);

	
	auto result = maxflow->filter( in_image );

	BOOST_REQUIRE(result);
	
	BOOST_CHECK_EQUAL ( result->get_size(), size);
	BOOST_CHECK_EQUAL ( result->get_pixel_type(), it_bit); 

	auto& bit_result = dynamic_cast<const C2DBitImage&>(*result); 
	
	auto iexp = expect.begin(); 
	for(auto ires = bit_result.begin(); ires != bit_result.end(); ++ires, ++iexp) {
		
		BOOST_CHECK_EQUAL(*ires, *iexp); 
	}
}


BOOST_AUTO_TEST_CASE( test_throw_missing_or_wrong_parameter )
{
	C2DUBImage in_image(C2DBounds(4,4));
	save_image("test.@", in_image); 

	BOOST_CHECK_THROW(BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow"),
			  invalid_argument);
	
	BOOST_CHECK_THROW(BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow:sink-flow=test.@"),
			  invalid_argument); 
	
	BOOST_CHECK_THROW(BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow:source-flow=test.@"),
			  invalid_argument); 

	auto maxflow = BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow:source-flow=test.@,sink-flow=test.@");

	// should throw because we don't accept non-float flow images 
	BOOST_CHECK_THROW(maxflow->filter(in_image), invalid_argument); 

}

BOOST_AUTO_TEST_CASE( test_throw_wrong_sizes )
{
	C2DFImage flow_image(C2DBounds(4,4));
	save_image("test.@", flow_image);

	C2DFImage wrong_image_size(C2DBounds(4,5));

	wrong_image_size(0,0) = 1.0;
	
	auto maxflow = BOOST_TEST_create_from_plugin<C2DMaxflowFilterPluginFactory>("maxflow:source-flow=test.@,sink-flow=test.@");

	BOOST_CHECK_THROW(maxflow->filter(wrong_image_size), invalid_argument);	

	C2DFImage flat_image(C2DBounds(4,4));

	BOOST_CHECK_THROW(maxflow->filter(flat_image), invalid_argument);
	
	
}

