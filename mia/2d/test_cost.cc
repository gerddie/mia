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

#include <mia/2d/cost.hh>
#include <mia/core/splinekernel.hh>
#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem; 
using namespace boost::unit_test;

CSplineKernelTestPath spline_kernel_set_test_path; 

BOOST_AUTO_TEST_CASE( test_2dimage_cost_avail )
{
	CPathNameArray searchpath; 
	
	searchpath.push_back(bfs::path("cost")); 

	C2DImageCostPluginHandler::set_search_path(searchpath); 
	
	const auto& handler = C2DImageCostPluginHandler::instance(); 

	BOOST_CHECK_EQUAL(handler.size(), 4u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "lsd mi ngf ssd "); 
}



