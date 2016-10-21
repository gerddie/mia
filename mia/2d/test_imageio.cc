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

static P2DImage create_test_image(int acquisition, int instance, const string& protocol) 
{
	C2DSIImage *image = new C2DSIImage(C2DBounds(1,2)); 
	P2DImage result(image); 
	(*image)(0,0) = acquisition; 
	(*image)(0,1) = instance;
	image->set_attribute(IDProtocolName, protocol); 
	image->set_attribute(IDAcquisitionNumber, PAttribute(new CIntAttribute(acquisition)));  
	image->set_attribute(IDSliceLocation, PAttribute(new CFloatAttribute(instance)));  
	return result; 
}

BOOST_AUTO_TEST_CASE( test_load_series ) 
{
	C2DImageIOPluginHandler::Instance::Data test_image_list1;
	test_image_list1.push_back(create_test_image(1,0,"proto1")); 
	test_image_list1.push_back(create_test_image(1,0,"proto2")); 
	test_image_list1.push_back(create_test_image(1,2,"proto1")); 
	test_image_list1.push_back(create_test_image(1,3,"proto1")); 
	test_image_list1.push_back(create_test_image(2,0,"proto2"));
	test_image_list1.push_back(create_test_image(1,0,"proto3"));

	C2DImageIOPluginHandler::Instance::Data test_image_list2;
	test_image_list2.push_back(create_test_image(2,4,"proto1")); 
	test_image_list2.push_back(create_test_image(1,1,"proto2")); 
	test_image_list2.push_back(create_test_image(1,5,"proto1")); 
	test_image_list2.push_back(create_test_image(1,6,"proto1")); 
	test_image_list2.push_back(create_test_image(3,2,"proto2"));
	test_image_list2.push_back(create_test_image(1,1,"proto3"));
	

	BOOST_REQUIRE(C2DImageIOPluginHandler::instance().save("test0.@", test_image_list1)); 
	BOOST_REQUIRE(C2DImageIOPluginHandler::instance().save("test1.@", test_image_list2)); 

	vector<string> filenames = {"test0.@","test1.@"}; 
		
	auto images = load_image_series(filenames); 
	// test three protocol types 
	BOOST_CHECK_EQUAL(images.size(), 3u); 
	
	// check first protocol 
	auto proto1 = images.find("proto1"); 
	BOOST_REQUIRE(proto1 !=  images.end()); 
	// proto 1 has 2 acquisitions 
	BOOST_CHECK_EQUAL(proto1->second.size(), 2u);
	
	BOOST_CHECK_EQUAL(proto1->second[0].size(), 5u);
	BOOST_CHECK_EQUAL(proto1->second[1].size(), 1u);
	

	auto proto2 = images.find("proto2"); 
	BOOST_REQUIRE(proto2 !=  images.end()); 
	BOOST_CHECK_EQUAL(proto2->second.size(), 3u);
	
	BOOST_CHECK_EQUAL(proto2->second[0].size(), 2u);
	BOOST_CHECK_EQUAL(proto2->second[1].size(), 1u);
	BOOST_CHECK_EQUAL(proto2->second[2].size(), 1u);

	auto proto3 = images.find("proto3"); 
	BOOST_REQUIRE(proto3 !=  images.end()); 

	BOOST_CHECK_EQUAL(proto3->second.size(), 1u);
	BOOST_CHECK_EQUAL(proto3->second[0].size(), 2u);

	BOOST_CHECK(images.find("proto4") ==  images.end()); 
	
}


BOOST_AUTO_TEST_CASE( test_load_bmp_8_uncompressed )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3.bmp");

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

	save_image("test_image-8u.bmp", test_image);

	auto test2_image = load_image2d("test_image-8u.bmp");
	
	const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u);
	BOOST_CHECK_EQUAL(img2(0,1), 128u);
	BOOST_CHECK_EQUAL(img2(1,1), 190u);
	BOOST_CHECK_EQUAL(img2(0,2), 229u);
	BOOST_CHECK_EQUAL(img2(1,2), 255u);

	unlink("test_image-8u.bmp"); 
	
}

BOOST_AUTO_TEST_CASE( test_load_bmp_8_compressed )
{
	auto test_image = load_image2d(MIA_SOURCE_ROOT"/testdata/gray100x2c.bmp");

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 100u);
	BOOST_CHECK_EQUAL(img.get_size().y, 2u);

	auto p = img.begin(); 
	
	for (unsigned x = 0; x < 52; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 163u);
	
	for (unsigned x = 52; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 139u); 

	for (unsigned x = 0; x < 47; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 100u); 
	for (unsigned x = 47; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 211u); 
	
}

BOOST_AUTO_TEST_CASE( test_load_bmp_4bit_compressed )
{
	auto test_image = load_image2d(MIA_SOURCE_ROOT"/testdata/gray100x2c-4bit.bmp");

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 100u);
	BOOST_CHECK_EQUAL(img.get_size().y, 2u);

	auto p = img.begin(); 
	
	for (unsigned x = 0; x < 52; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 2u);

	BOOST_CHECK_EQUAL(*p++, 1u);
	BOOST_CHECK_EQUAL(*p++, 3u);
	BOOST_CHECK_EQUAL(*p++, 1u);
	BOOST_CHECK_EQUAL(*p++, 0u);
	BOOST_CHECK_EQUAL(*p++, 1u);
	BOOST_CHECK_EQUAL(*p++, 0u);

	
	for (unsigned x = 58; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 1u); 

	for (unsigned x = 0; x < 47; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u);

	
	for (unsigned x = 47; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 3u); 
	
}

BOOST_AUTO_TEST_CASE( test_load_bmp_4bit_uncompressed )
{
	auto test_image = load_image2d(MIA_SOURCE_ROOT"/testdata/gray100x2uc-4bit.bmp");

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 100u);
	BOOST_CHECK_EQUAL(img.get_size().y, 2u);

	auto p = img.begin(); 
	
	for (unsigned x = 0; x < 52; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 2u);
	
	for (unsigned x = 52; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 1u); 

	for (unsigned x = 0; x < 47; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 
	for (unsigned x = 47; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 3u); 
	
}

BOOST_AUTO_TEST_CASE( test_load_save_bmp_1bit_uc )
{
	auto test_image = load_image2d(MIA_SOURCE_ROOT"/testdata/binary100x2uc.bmp");

	const C2DBitImage& img = dynamic_cast<const C2DBitImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 100u);
	BOOST_CHECK_EQUAL(img.get_size().y, 2u);

	auto p = img.begin(); 
	
	for (unsigned x = 0; x < 55; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u);

	BOOST_CHECK_EQUAL(*p++, 1u);
	BOOST_CHECK_EQUAL(*p++, 0u);
	BOOST_CHECK_EQUAL(*p++, 1u);

	
	for (unsigned x = 58; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 

	for (unsigned x = 0; x < 47; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 1u);

	
	for (unsigned x = 47; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 


	save_image("test_image-bit.bmp", test_image);

	auto test2_image = load_image2d("test_image-bit.bmp");


	const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 100u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 2u);

	p = img2.begin(); 
	
	for (unsigned x = 0; x < 55; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u);

	BOOST_CHECK_EQUAL(*p++, 1u);
	BOOST_CHECK_EQUAL(*p++, 0u);
	BOOST_CHECK_EQUAL(*p++, 1u);

	
	for (unsigned x = 58; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 

	for (unsigned x = 0; x < 47; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 1u);

	
	for (unsigned x = 47; x < 100; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 

	unlink("test_image-bit.bmp"); 
}

BOOST_AUTO_TEST_CASE( test_load_bmp_8_compressed_with_jumps )
{
	// this test should check whether the jump inside a bitmap file is propperly
	// executed, but currently the image is not encoded with such a jump. 
	
	auto test_image = load_image2d(MIA_SOURCE_ROOT"/testdata/gray20x20c.bmp");

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 20u);
	BOOST_CHECK_EQUAL(img.get_size().y, 20u);

	auto p = img.begin(); 
	unsigned x; 
	for (x = 0; x < 9; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 99u);

	for (; x < 8*20+6; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u);

	for (; x < 9*20; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 121u);
	
	for (; x < 14*20; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u);

	for (; x < 14*20 + 10; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 138u);
	
	for (; x < 20*20; ++x, ++p)
		BOOST_CHECK_EQUAL(*p, 0u); 

}

BOOST_AUTO_TEST_CASE( test_load_save_rgb8_bmp )
{
	
        vector<unsigned char> test_data{206, 89, 97, 71, 99, 67, 192, 205, 52,
                        28, 31, 98, 94, 27, 204, 232, 18, 214};
        
        const auto& io = C2DRGBImageIOPluginPluginHandler::instance(); 

        auto test_image = io.load(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.bmp");

        const CRGB2DImage& img = *test_image;

        BOOST_CHECK_EQUAL(img.get_size().x, 3);
        BOOST_CHECK_EQUAL(img.get_size().y, 2);
        
        auto pixels = img.pixel();
        for (int i = 0; i < 18; ++i) {
		cvdebug() << i << ":" <<  (int)pixels[i] << " exp " << (int)test_data[i] << "\n"; 
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        BOOST_REQUIRE(save_image("test_image_rgb.bmp", *test_image));
        
        auto test_image2 = io.load("test_image_rgb.bmp");

        const CRGB2DImage& img2 = *test_image2;

        BOOST_CHECK_EQUAL(img2.get_size().x, 3);
        BOOST_CHECK_EQUAL(img2.get_size().y, 2);
        
        pixels = img2.pixel();
        for (int i = 0; i < 18; ++i) {
                BOOST_CHECK_EQUAL(pixels[i], test_data[i]); 
        }

        unlink("test_image_rgb.bmp"); 
}
