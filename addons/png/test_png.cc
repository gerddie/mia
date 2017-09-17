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


#include <mia/internal/autotest.hh>
#include <boost/filesystem/path.hpp>

#include <mia/core/attribute_names.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/rgbimageio.hh>

NS_MIA_USE
using namespace std; 
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem; 


BOOST_AUTO_TEST_CASE( test_load_save_8bit_gray )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3.png");


        auto test_image = load_image2d(filename);

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0),   0u);
	BOOST_CHECK_EQUAL(img(1,0),  63u);
	BOOST_CHECK_EQUAL(img(0,1), 128u);
	BOOST_CHECK_EQUAL(img(1,1), 190u);
	BOOST_CHECK_EQUAL(img(0,2), 229u);
	BOOST_CHECK_EQUAL(img(1,2), 255u);

	save_image("test_image.png", test_image);

	auto test2_image = load_image2d("test_image.png");
	
	const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u);
	BOOST_CHECK_EQUAL(img2(0,1), 128u);
	BOOST_CHECK_EQUAL(img2(1,1), 190u);
	BOOST_CHECK_EQUAL(img2(0,2), 229u);
	BOOST_CHECK_EQUAL(img2(1,2), 255u);
        unlink("test_image.png"); 	
}

BOOST_AUTO_TEST_CASE( test_load_save_1bit_gray )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-1.png");


        auto test_image = load_image2d(filename);

	const C2DBitImage& img = dynamic_cast<const C2DBitImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0), 0u);
	BOOST_CHECK_EQUAL(img(1,0), 0u);
	BOOST_CHECK_EQUAL(img(0,1), 0u);
	BOOST_CHECK_EQUAL(img(1,1), 1u);
	BOOST_CHECK_EQUAL(img(0,2), 1u);
	BOOST_CHECK_EQUAL(img(1,2), 1u);

	save_image("test_image.png", test_image);

	auto test2_image = load_image2d("test_image.png");
	
	const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0), 0u);
	BOOST_CHECK_EQUAL(img(1,0), 0u);
	BOOST_CHECK_EQUAL(img(0,1), 0u);
	BOOST_CHECK_EQUAL(img(1,1), 1u);
	BOOST_CHECK_EQUAL(img(0,2), 1u);
	BOOST_CHECK_EQUAL(img(1,2), 1u);

        unlink("test_image.png"); 
	
}

BOOST_AUTO_TEST_CASE( test_load_save_16bit_gray )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-16.png");


        auto test_image = load_image2d(filename);

	const C2DUSImage& img = dynamic_cast<const C2DUSImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0),   0u);
	BOOST_CHECK_EQUAL(img(1,0),  63u * 256);
	BOOST_CHECK_EQUAL(img(0,1), 128u * 256);
	BOOST_CHECK_EQUAL(img(1,1), 190u * 256);
	BOOST_CHECK_EQUAL(img(0,2), 229u * 256);
	BOOST_CHECK_EQUAL(img(1,2), 255u * 256);

	save_image("test_image.png", test_image);

	auto test2_image = load_image2d("test_image.png");
	
	const C2DUSImage& img2 = dynamic_cast<const C2DUSImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u * 256);
	BOOST_CHECK_EQUAL(img2(0,1), 128u * 256);
	BOOST_CHECK_EQUAL(img2(1,1), 190u * 256);
	BOOST_CHECK_EQUAL(img2(0,2), 229u * 256);
	BOOST_CHECK_EQUAL(img2(1,2), 255u * 256);

        unlink("test_image.png"); 
}

BOOST_AUTO_TEST_CASE( test_load_save_8bit_rgb )
{
        vector<unsigned char> test_data{206, 89, 97, 71, 99, 67, 192, 205, 52,
                        28, 31, 98, 94, 27, 204, 232, 18, 214};
        
        const auto& io = C2DRGBImageIOPluginPluginHandler::instance(); 

        auto test_image = io.load(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.png");

        const CRGB2DImage& img = *test_image;

        BOOST_CHECK_EQUAL(img.get_size().x, 3);
        BOOST_CHECK_EQUAL(img.get_size().y, 2);
        
        auto pixels = img.pixel();
        for (int i = 0; i < 18; ++i) {
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        BOOST_REQUIRE(save_image("test_image_rgb.png", *test_image));
        
        auto test_image2 = io.load("test_image_rgb.png");

        const CRGB2DImage& img2 = *test_image2;

        BOOST_CHECK_EQUAL(img2.get_size().x, 3);
        BOOST_CHECK_EQUAL(img2.get_size().y, 2);
        
        pixels = img2.pixel();
        for (int i = 0; i < 18; ++i) {
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        unlink("test_image_rgb.png"); 
        
}

BOOST_AUTO_TEST_CASE( test_rejects )
{
        const auto& io = C2DRGBImageIOPluginPluginHandler::instance(); 

	BOOST_CHECK_THROW(io.load(MIA_SOURCE_ROOT"/testdata/gray2x3.png"),
			  invalid_argument);
	
	BOOST_CHECK_THROW(io.load(MIA_SOURCE_ROOT"/testdata/nonexistent"),
			  runtime_error); 

	
	BOOST_CHECK_THROW(load_image2d(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.png"),
			  invalid_argument);
}
