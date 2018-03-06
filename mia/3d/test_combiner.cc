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

#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/3d/filter.hh>
#include <mia/core/history.hh>
#include <mia/core/spacial_kernel.hh>

NS_MIA_USE
using namespace std;
namespace bfs =::boost::filesystem;

static void setup_filter_search_path()
{
       list< bfs::path> searchpath;

       if (bfs::path::default_name_check_writable())
              bfs::path::default_name_check(bfs::portable_posix_name);

       searchpath.push_back(bfs::path("3d") / bfs::path("combiner"));
       C3DImageCombinerPluginHandler::set_search_path(searchpath);
}

static void test_3dcombiner(const C3DImageCombinerPluginHandler::value_type& i)
{
       BOOST_CHECK_MESSAGE(i.second->test(true), i.second->get_long_name());
}


void add_3dcombiner_plugin_tests( boost::unit_test::test_suite *suite)
{
       setup_filter_search_path();
       suite->add( BOOST_PARAM_TEST_CASE(&test_3dcombiner,
                                         C3DImageCombinerPluginHandler::instance().begin(),
                                         C3DImageCombinerPluginHandler::instance().end()
                                        ));
}
