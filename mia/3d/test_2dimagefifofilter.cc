/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define BOOST_TEST_MODULE 2DIMAGEFIFOFILTER_TEST
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK
#include <cassert>
#include <iostream>
#include <cmath>
#include <numeric>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


#include <mia/core/cmdlineparser.hh>
#include <mia/3d/2dimagefifofilter.hh>

using namespace std;
using namespace mia;
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_found_filters )
{
	list< bfs::path> searchpath;
	searchpath.push_back( bfs::path("fifof"));
	C2DFifoFilterPluginHandler::set_search_path(searchpath);
	BOOST_CHECK_EQUAL(C2DFifoFilterPluginHandler::instance().size(), 8);

	BOOST_CHECK_EQUAL(C2DFifoFilterPluginHandler::instance().get_plugin_names(),
			  "close dilate erode gauss median mlv open regiongrow ");

}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	mia::CCmdOptionList().parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
