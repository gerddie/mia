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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <boost/filesystem/path.hpp>
#include <mia/2d/filter.hh>


NS_MIA_USE
using namespace std;
namespace bfs=boost::filesystem; 

BOOST_AUTO_TEST_CASE( test_load_plugins ) 
{	
	CPathNameArray plugpath;
	plugpath.push_back(bfs::path("combiner"));
	C2DImageCombinerPluginHandler::set_search_path(plugpath);

	const C2DImageCombinerPluginHandler::Instance& handler = C2DImageCombinerPluginHandler::instance(); 
	BOOST_CHECK_EQUAL(handler.size(), 5u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "absdiff add div mul sub ");
}
