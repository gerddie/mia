/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <mia/2d/imageio.hh>
#include <mia/2d/imageiotest.hh>
#include <mia/2d/vfio.hh>

NS_MIA_USE
using namespace std;
using namespace boost;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

struct PathInitFixture {
	PathInitFixture(); 
}; 

PathInitFixture::PathInitFixture()
{
	CPathNameArray searchpath;
	searchpath.push_back(bfs::path("."));

	C2DImageIOPluginHandler::set_search_path(searchpath);
	C2DVFIOPluginHandler::set_search_path(searchpath);
}

BOOST_FIXTURE_TEST_CASE(test_2dimage_plugin_handler, PathInitFixture)
{
	const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance();
	BOOST_REQUIRE(handler.size() == 2u);
	BOOST_REQUIRE(handler.get_plugin_names() == "datapool exr ");
}

BOOST_FIXTURE_TEST_CASE(test_2dvf_plugin_handler, PathInitFixture)
{
	const C2DVFIOPluginHandler::Instance& handler = C2DVFIOPluginHandler::instance();
	BOOST_REQUIRE(handler.size() == 2u);
	BOOST_REQUIRE(handler.get_plugin_names() == "datapool exr ");
}

BOOST_FIXTURE_TEST_CASE(test_2dimage_plugin, PathInitFixture)
{
	test_2dimageio_plugins(); 
}
