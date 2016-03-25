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

#include <algorithm>


#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/3d/vfio.hh>
#include <mia/3d/vfiotest.hh>

NS_MIA_USE
using namespace std; 
namespace bfs=::boost::filesystem; 

static void prepare_handler()
{
	std::list< ::boost::filesystem::path> searchpath; 
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(bfs::portable_posix_name); 
	searchpath.push_back(bfs::path("3d") / bfs::path("io")); 
	searchpath.push_back(bfs::path("io")); 

	C3DVFIOPluginHandler::set_search_path(searchpath); 	
}



static void test_3dvfio_plugin_avail()
{
	const C3DVFIOPluginHandler::Instance&  handler = C3DVFIOPluginHandler::instance(); 

	BOOST_CHECK(handler.size() == 1); 
	BOOST_CHECK(handler.get_plugin_names() == "vtk ");

}

void add_3dvfio_tests( boost::unit_test::test_suite* suite)
{
	prepare_handler();

	suite->add( BOOST_TEST_CASE(&test_3dvfio_plugin_avail)); 
	add_3dvfio_plugin_tests(suite);
}
