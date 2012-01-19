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

#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filtertest.hh>
#include <mia/2d/2dfilter.hh>



NS_MIA_BEGIN
using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace boost::unit_test;

EXPORT_2DTEST void imagefiltertest2d_prepare_plugin_path()
{
	list< bfs::path> filtersearchpath;
	filtersearchpath.push_back( bfs::path("2d") / bfs::path("filter"));
	filtersearchpath.push_back( bfs::path("filter"));
	C2DFilterPluginHandler::set_search_path(filtersearchpath);

	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("core")/bfs::path("spacialkernel"));
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("core")/bfs::path("spacialkernel"));

	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);
}


static void test_2dfilter(const C2DFilterPluginHandler::value_type& i)
{
	cvmsg() << "Testing: " << i.second->get_long_name() << "\n";
	if (!i.second->test(true))
		BOOST_FAIL(i.second->get_long_name());
}

void EXPORT_2DTEST add_2dfilter_plugin_test(test_suite* suite)
{
	suite->add( BOOST_PARAM_TEST_CASE(&test_2dfilter,
					  C2DFilterPluginHandler::instance().begin(),
					  C2DFilterPluginHandler::instance().end()
					  ));
}


NS_MIA_END


