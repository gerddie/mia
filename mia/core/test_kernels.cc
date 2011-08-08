/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core/spacial_kernel.hh>
#include <mia/core/history.hh>

NS_MIA_USE
using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_spacial_kernels )
{
	list< bfs::path> searchpath;

	searchpath.push_back( bfs::path("core") / bfs::path("spacialkernel"));
	searchpath.push_back( bfs::path("spacialkernel"));

	C1DSpacialKernelPluginHandler::set_search_path(searchpath);

	const C1DSpacialKernelPluginHandler::Instance& fh = C1DSpacialKernelPluginHandler::instance();

	for (C1DSpacialKernelPluginHandler::Instance::const_iterator i = fh.begin();
	     i != fh.end(); ++i)
		i->second->test(true);
}




