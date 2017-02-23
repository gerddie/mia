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

#include <mia/2d/cost.hh>
#include <mia/core/splinekernel.hh>
#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem; 
using namespace boost::unit_test;
using std::set; 
using std::string; 

BOOST_AUTO_TEST_CASE( test_2dimage_cost_avail )
{
	set<string> test_data = {"lncc","lsd", "mi", "ngf", "ncc", "ssd", "ssd-automask"}; 
	test_pluginsets(C2DImageCostPluginHandler::instance().get_set(), test_data); 
}



