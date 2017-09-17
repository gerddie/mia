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
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3.tif");


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

	save_image("test_image.tif", test_image);

	auto test2_image = load_image2d("test_image.tif");
	
	const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u);
	BOOST_CHECK_EQUAL(img2(0,1), 128u);
	BOOST_CHECK_EQUAL(img2(1,1), 190u);
	BOOST_CHECK_EQUAL(img2(0,2), 229u);
	BOOST_CHECK_EQUAL(img2(1,2), 255u);
        unlink("test_image.tif"); 	
}

BOOST_AUTO_TEST_CASE( test_load_save_1bit_gray )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-1.tif");


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

	save_image("test_image.tif", test_image);

	auto test2_image = load_image2d("test_image.tif");
	
	const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0), 0u);
	BOOST_CHECK_EQUAL(img(1,0), 0u);
	BOOST_CHECK_EQUAL(img(0,1), 0u);
	BOOST_CHECK_EQUAL(img(1,1), 1u);
	BOOST_CHECK_EQUAL(img(0,2), 1u);
	BOOST_CHECK_EQUAL(img(1,2), 1u);

        unlink("test_image.tif"); 
	
}

BOOST_AUTO_TEST_CASE( test_load_save_16bit_gray )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-16.tif");


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

	save_image("test_image.tif", test_image);

	auto test2_image = load_image2d("test_image.tif");
	
	const C2DUSImage& img2 = dynamic_cast<const C2DUSImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u * 256);
	BOOST_CHECK_EQUAL(img2(0,1), 128u * 256);
	BOOST_CHECK_EQUAL(img2(1,1), 190u * 256);
	BOOST_CHECK_EQUAL(img2(0,2), 229u * 256);
	BOOST_CHECK_EQUAL(img2(1,2), 255u * 256);

        unlink("test_image.tif"); 
}

#ifdef USE_TIFF_RGB_IO
BOOST_AUTO_TEST_CASE( test_load_save_8bit_rgb )
{
        vector<unsigned char> test_data{206, 89, 97, 71, 99, 67, 192, 205, 52,
                        28, 31, 98, 94, 27, 204, 232, 18, 214};
        
        const auto& io = C2DRGBImageIOPluginPluginHandler::instance(); 

        auto test_image = io.load(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.tif");

        const CRGB2DImage& img = *test_image;

        BOOST_CHECK_EQUAL(img.get_size().x, 3);
        BOOST_CHECK_EQUAL(img.get_size().y, 2);
        
        auto pixels = img.pixel();
        for (int i = 0; i < 18; ++i) {
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        BOOST_REQUIRE(save_image("test_image_rgb.tif", *test_image));
        
        auto test_image2 = io.load("test_image_rgb.tif");

        const CRGB2DImage& img2 = *test_image2;

        BOOST_CHECK_EQUAL(img2.get_size().x, 3);
        BOOST_CHECK_EQUAL(img2.get_size().y, 2);
        
        pixels = img2.pixel();
        for (int i = 0; i < 18; ++i) {
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        unlink("test_image_rgb.tif"); 
        
}
#endif 

BOOST_AUTO_TEST_CASE( test_rejects )
{
	try {

	BOOST_CHECK_THROW(load_image2d(MIA_SOURCE_ROOT"/testdata/nonexistent.tif"),
			  std::runtime_error); 
	}
	catch (std::runtime_error& x) {
		cvwarn() << "Expected exception thrown but BOOST_CHECK_THROW didn't catch it\n";
	}
}


BOOST_AUTO_TEST_CASE( test_load_save_8bit_gray_multiframe )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-multiframe.tif");

	const auto& io = C2DImageIOPluginHandler::instance(); 
	
        auto test_images = io.load(filename);

	BOOST_CHECK_EQUAL(test_images->size(), 2u);

	vector<vector<unsigned char>> expect{
		{ 0, 63,128,190,229,255},
		{68,212,135,190,194,184}};   
			

	for (unsigned i = 0; i < test_images->size(); ++i) {
		const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*(*test_images)[i]); 
		BOOST_CHECK_EQUAL(img.get_size().x, 2u);
		BOOST_CHECK_EQUAL(img.get_size().y, 3u);
		
		BOOST_CHECK_EQUAL(img(0,0), expect[i][0]);
		BOOST_CHECK_EQUAL(img(1,0), expect[i][1]);
		BOOST_CHECK_EQUAL(img(0,1), expect[i][2]);
		BOOST_CHECK_EQUAL(img(1,1), expect[i][3]);
		BOOST_CHECK_EQUAL(img(0,2), expect[i][4]);
		BOOST_CHECK_EQUAL(img(1,2), expect[i][5]);
	}

	BOOST_REQUIRE(io.save("test_image_multi.tif", *test_images));

	auto test2_images = io.load("test_image_multi.tif");
	BOOST_CHECK_EQUAL(test2_images->size(), 2u);

	for (unsigned i = 0; i < test_images->size(); ++i) {
		const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*(*test2_images)[i]);
		BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
		BOOST_CHECK_EQUAL(img2.get_size().y, 3u);
		
		BOOST_CHECK_EQUAL(img2(0,0), expect[i][0]);
		BOOST_CHECK_EQUAL(img2(1,0), expect[i][1]);
		BOOST_CHECK_EQUAL(img2(0,1), expect[i][2]);
		BOOST_CHECK_EQUAL(img2(1,1), expect[i][3]);
		BOOST_CHECK_EQUAL(img2(0,2), expect[i][4]);
		BOOST_CHECK_EQUAL(img2(1,2), expect[i][5]);
	}

	unlink("test_image_multi.tif"); 	
}
