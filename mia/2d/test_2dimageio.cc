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
#include <boost/filesystem/path.hpp>

#include <mia/core/msgstream.hh>
#include <mia/2d/2dimageio.hh>

NS_MIA_USE
using namespace std; 
using namespace boost; 
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem; 

C2DImageIOPluginHandlerTestPath test_imageio_path; 

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
	

	C2DImageIOPluginHandler::instance().save("test0.@", test_image_list1); 
	C2DImageIOPluginHandler::instance().save("test1.@", test_image_list2); 
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

