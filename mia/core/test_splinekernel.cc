/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>
#include <mia/core/splinekernel.hh>

namespace bfs=::boost::filesystem; 


NS_MIA_USE
using namespace std;
using namespace boost;

CSplineKernelTestPath init_path; 

BOOST_AUTO_TEST_CASE( test_plugin_availability ) 
{
	const auto& handler = CSplineKernelPluginHandler::instance();

	BOOST_CHECK_EQUAL(handler.size(), 2u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "bspline omoms "); 

}
