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

#include <climits>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <mia/3d/cost.hh>


NS_MIA_USE
using namespace std; 
namespace bfs=::boost::filesystem; 

static void prepare_plugin_path()
{
	list< bfs::path> searchpath; 
	
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 

	searchpath.push_back(bfs::path("3d") / bfs::path("cost")); 
	searchpath.push_back(bfs::path("cost")); 

	C3DImageCostPluginHandler::set_search_path(searchpath); 
}

static void test_3dimage_cost_avail() 
{
	const C3DImageCostPluginHandler::Instance& fh = C3DImageCostPluginHandler::instance();
	BOOST_CHECK(fh.size() == 4); 
	BOOST_CHECK(fh.get_plugin_names() == "mi ncc ngf ssd ");
}

static void test_3dimage_cost(const C3DImageCostPluginHandler::value_type& i)
{
	BOOST_CHECK_MESSAGE(i.second->test(true), i.second->get_long_name()); 
}

void add_3dimagecost_tests(test_suite* suite)
{
	prepare_plugin_path(); 
	suite->add( BOOST_TEST_CASE( &test_3dimage_cost_avail)); 
}
