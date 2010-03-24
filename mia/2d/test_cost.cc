/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/2d/cost.hh>


NS_MIA_USE
using namespace std; 
using namespace boost;
namespace bfs=::boost::filesystem; 

using namespace boost::unit_test;

static void prepare_plugin_path()
{
	list< bfs::path> searchpath; 
	
	searchpath.push_back(bfs::path("2d") / bfs::path("cost")); 
	searchpath.push_back(bfs::path("cost")); 

	C2DImageCostPluginHandler::set_search_path(searchpath); 
}

static void test_2dimage_cost_avail() 
{
	const C2DImageCostPluginHandler::Instance& fh = C2DImageCostPluginHandler::instance();
	BOOST_CHECK(fh.size() == 2); 
	BOOST_CHECK(fh.get_plugin_names() == "ssd ssddf ");
}

static void test_2dimage_cost()
{
	const C2DImageCostPluginHandler::Instance& fh = C2DImageCostPluginHandler::instance();

	for (C2DImageCostPluginHandler::Instance::const_iterator i = fh.begin(); 
	     i != fh.end(); ++i) {
		cvmsg() << "Testing: " << i->second->get_long_name() << "\n"; 
		BOOST_CHECK_MESSAGE(i->second->test(true), i->second->get_long_name()); 
	}
}

void add_2dimagecost_tests(test_suite* suite)
{
	prepare_plugin_path(); 

	suite->add( BOOST_TEST_CASE( &test_2dimage_cost_avail)); 
	suite->add( BOOST_TEST_CASE( &test_2dimage_cost));
}
