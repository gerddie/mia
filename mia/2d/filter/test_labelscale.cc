/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/2d/filter/labelscale.hh>

using namespace labelscale_2dimage_filter; 
using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_labelscale_downscale ) 
{
	const C2DBounds in_size(10, 12); 
        
	const unsigned char in_image[120] = {
                1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 
                1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 
                1, 1, 1, 2, 2, 3, 3, 3, 3, 4,
                5, 5, 5, 2, 2, 2, 3, 3, 4, 4, 
                5, 5, 2, 2, 2, 3, 3, 3, 4, 4, 
                5, 5, 5, 2, 7, 2, 3, 3, 3, 3, 
                5, 6, 5, 7, 7, 7, 7, 3, 3, 3, 
                6, 6, 6, 7, 7, 7, 7, 3, 3, 3, 
                6, 6, 8, 7, 7, 7, 7, 3, 3, 3, 
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3, 
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3,
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3,

	}; 

	const C2DBounds out_size(5, 3); 
	const unsigned char out_image[15] = {
                1,    2,    2,    3,    3, 
                5,    2,    7,    3,    3, 
                8,    8,    7,    7,    3
	}; 

        auto f = BOOST_TEST_create_from_plugin<C2DLabelscaleFilterPluginFactory>("labelscale:out-size=[5,3]"); 
 

        C2DUBImage fimage(in_size, in_image );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));

        P2DImage scaled = f->filter(fimage);
        
        BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(5,3));

	const C2DUBImage& fscaled = dynamic_cast<const C2DUBImage& >(*scaled);
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(5,3));

	BOOST_CHECK_EQUAL(fscaled.get_pixel_size(), C2DFVector(4.0f, 12.f));
        
        for (size_t i = 0; i < 15; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << out_image[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], out_image[i]); 
	}
       
}



BOOST_AUTO_TEST_CASE ( test_labelscale_upscale ) 
{
	const C2DBounds out_size(10, 12); 
        
	const unsigned char out_image[120] = {
                1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 
                1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 
                1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 

                5, 5, 2, 2, 7, 7, 3, 3, 3, 3, 
                5, 5, 2, 2, 7, 7, 3, 3, 3, 3, 
                5, 5, 2, 2, 7, 7, 3, 3, 3, 3, 
                5, 5, 2, 2, 7, 7, 3, 3, 3, 3, 

                8, 8, 8, 8, 7, 7, 7, 7, 3, 3, 
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3, 
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3,
                8, 8, 8, 8, 7, 7, 7, 7, 3, 3,

	}; 

	const C2DBounds in_size(5, 3); 
	const unsigned char in_image[15] = {
                1,    2,    2,    3,    3, 
                5,    2,    7,    3,    3, 
                8,    8,    7,    7,    3
	}; 

        auto f = BOOST_TEST_create_from_plugin<C2DLabelscaleFilterPluginFactory>("labelscale:out-size=[10,12]"); 
 

        C2DUBImage fimage(in_size, in_image );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));

        P2DImage scaled = f->filter(fimage);
        
        BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(10,12));

	const C2DUBImage& fscaled = dynamic_cast<const C2DUBImage& >(*scaled);
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(10,12));

	BOOST_CHECK_EQUAL(fscaled.get_pixel_size(), C2DFVector(1.0f, 0.75f));
        
        for (size_t i = 0; i < 120; ++i) {
		cvdebug() << i << ":" << int(fscaled[i]) << " - " << int(out_image[i]) << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], out_image[i]); 
	}
       
}

