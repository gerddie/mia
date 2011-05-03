/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dimageiotest.hh>
#include <mia/core/filter.hh>

NS_MIA_USE
using namespace std; 
using namespace boost; 
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem; 

typedef C2DImageIOPluginHandler::Instance::Interface Interface; 

// remove the warnings: 
template <typename T> 
struct check_pixels {
	static bool apply(const T2DImage<T>& image)  {
		BOOST_CHECK(image(0,0) == 0); 
		BOOST_CHECK(image(1,0) == 140); 
		BOOST_CHECK(image(0,1) == 90); 
		BOOST_CHECK(image(1,1) == 255); 
		return true;
	}
}; 

template <> 
struct check_pixels<signed char> {
	static bool apply(const T2DImage<signed char>& image)  {
		BOOST_FAIL("expect an unsiged valued image"); 
		return false;
	}
}; 

struct CCheckFilter: public TFilter<bool> {
	template <typename T> 
	CCheckFilter::result_type operator ()(const T2DImage<T>& image)const {
		return check_pixels<T>::apply(image); 
	}
};



static void handler_setup()
{
	std::list< bfs::path> searchpath; 

	searchpath.push_back( bfs::path("2d") / bfs::path("io")); 
	searchpath.push_back(bfs::path("io")); 
	C2DImageIOPluginHandler::set_search_path(searchpath); 	
}

#if 0
static void test_2dimage_plugin_handler()
{
	const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance(); 
	BOOST_REQUIRE(handler.size() == 4); 
	BOOST_REQUIRE(handler.get_plugin_names() == "bmp datapool png tif ");
}


static void test_2dimage_io_png()
{
	
	const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance(); 
	bfs::path file = bfs::path(SOURCE_ROOT) / bfs::path("mia") / bfs::path("2d")/bfs::path("gray8.png"); 
	C2DImageIOPluginHandler::Instance::PData gray8 = handler.load(file.native_file_string()); 

	BOOST_REQUIRE(gray8.get()); 
	BOOST_REQUIRE(gray8->size() == 1); 
	
	// test the image
	CCheckFilter check; 
	BOOST_CHECK(filter(check, **gray8->begin())); 
}
#endif

void add_2dimageio_tests(test_suite* test)
{	
	handler_setup(); 

#if 0
	test->add( BOOST_TEST_CASE( &test_2dimage_plugin_handler));
	test->add( BOOST_TEST_CASE( &test_2dimage_io_png)); 
#endif

	test->add( BOOST_TEST_CASE( &test_2dimageio_plugins));

}
