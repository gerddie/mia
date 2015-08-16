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

#include <mia/internal/plugintester.hh>


using namespace std;
using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_2d_simple_maxflow )
{
	// one source, one sink and a pretty simple 4x4 image

	// sink and source


	vector<unsigned char> image = {
		255, 255, 245, 233,
		200, 120, 120, 100,
		20,   30,  40,  30,
		40,   20,  50,  80
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

	C2DImage *fimage = new C2DFImage(size, image);
	(*fimage)(simk) = 1.0f;  
	P2DImage sink_image(fimage);

	fimage = new C2DFImage(size, image);
	(*fimage)(source) = 1.0f; 
	P2DImage source_image(fimage);

       	auto maxflow = BOOST_TEST_create_from_plugin<C2DMaxflowFilterPlugin>("maxflow:src-flow=src.@,sink-flow=sink.@");

	save_image("src.@", source_image);
	save_image("sink.@", sink_image);

	
	auto result = maxflow->filter( in_image );

	BOOST_REQUIRE(result);
	
	BOOST_CHECK_EQUAL ( result->get_size(), size);
	BOOST_CHECK_EQUAL ( result->get_pixel_type(), it_bit); 

	auto& bit_result = dynamic_cast<const C2DBitImage&>(*result); 
	
	auto iexp = expect.begin(); 
	for(auto ires = bit_result.begin(); ires != bit_result.end(); ++ires, +iexp) {
		BOOST_CHECK_EQUAL(*ires, *iexp); 
	}
}

