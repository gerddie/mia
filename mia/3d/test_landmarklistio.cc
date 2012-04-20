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


#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/internal/pluginsettest.hh>
#include <mia/3d/landmarklistio.hh>

NS_MIA_USE
using namespace std;

BOOST_AUTO_TEST_CASE( test_available_plugins ) 
{
	set<string> expected_plugins = {"lmx", "datapool"};
	test_availabe_plugins(C3DImageIOPluginHandler::instance(), expected_plugins); 
}

