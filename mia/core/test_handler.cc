/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#define BOOST_TEST_DYN_LINK

#include <iostream>
#include <climits>

#include <boost/any.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core/handler.hh>
#include <mia/core/testplugin.hh>

NS_MIA_USE
using namespace std;
namespace bfs = ::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_dummy_plugin_handler )
{
	CTestPluginHandler::set_search_path({bfs::path("testplug")});

	const CTestPluginHandler::Instance& handler = CTestPluginHandler::instance();


	BOOST_CHECK(handler.size() == 3);

	BOOST_CHECK(handler.get_plugin_names() == "dummy1 dummy2 dummy3 ");

	BOOST_CHECK(handler.get_plugin("dummy3")->has_property(test_property));
	BOOST_CHECK(!handler.get_plugin("dummy1")->has_property(test_property));

}

BOOST_AUTO_TEST_CASE( test_windows_interface )
{
	bfs::path test_path = bfs::path("c:") / bfs::path("windows");

}


