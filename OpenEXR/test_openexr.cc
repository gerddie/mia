/* -*- mia-c++  -*-
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <climits>

#define BOOST_TEST_DYN_LINK

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/cmdlineparser.hh>

#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dimageiotest.hh>

#include <mia/2d/2dvfio.hh>

NS_MIA_USE
using namespace std; 
using namespace boost; 
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem; 

static void handler_setup()
{
	std::list< bfs::path> searchpath; 
	searchpath.push_back(bfs::path(".")); 
	
	C2DImageIOPluginHandler::set_search_path(searchpath);
	C2DVFIOPluginHandler::set_search_path(searchpath);
}

static void test_2dimage_plugin_handler()
{
	const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance(); 
	BOOST_REQUIRE(handler.size() == 2); 
	BOOST_REQUIRE(handler.get_plugin_names() == "datapool exr ");
}

static void test_2dvf_plugin_handler()
{
	const C2DVFIOPluginHandler::Instance& handler = C2DVFIOPluginHandler::instance(); 
	BOOST_REQUIRE(handler.size() == 2); 
	BOOST_REQUIRE(handler.get_plugin_names() == "datapool exr ");
}

extern void add_2dvfio_tests(test_suite* suite); 

bool init_unit_test_suite( ) 
{


	handler_setup(); 
	
	test_suite *suite = &framework::master_test_suite(); 

	suite->add( BOOST_TEST_CASE( &test_2dimage_plugin_handler));
	suite->add( BOOST_TEST_CASE( &test_2dimageio_plugins)); 	
	
	suite->add( BOOST_TEST_CASE( &test_2dvf_plugin_handler )); 
	//add_2dvfio_tests(test);
	return true; 
}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	CCmdOptionList().parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}
