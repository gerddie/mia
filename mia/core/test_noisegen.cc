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

#include <cassert>
#include <iostream>
#include <cmath>

#include <mia/internal/autotest.hh>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/noisegen.hh>

using namespace std; 
using namespace mia; 
using namespace boost;
namespace bfs=::boost::filesystem; 

BOOST_AUTO_TEST_CASE( test_plugins )	
{
	list< bfs::path> searchpath; 

	searchpath.push_back( bfs::path("core") / bfs::path("noise"));
	searchpath.push_back( bfs::path("noise"));
	
	CNoiseGeneratorPluginHandler::set_search_path(searchpath); 

	const CNoiseGeneratorPluginHandler::Instance& fh = CNoiseGeneratorPluginHandler::instance();

	for (CNoiseGeneratorPluginHandler::Instance::const_iterator i = fh.begin(); 
	     i != fh.end(); ++i)
		BOOST_CHECK(i->second->test(true)); 
}

BOOST_AUTO_TEST_CASE( test_factory_trait ) 
{
	FactoryTrait<CNoiseGeneratorPluginHandler::ProductPtr>::type::Instance help = 
		CNoiseGeneratorPluginHandler::instance();
	BOOST_CHECK(!help.empty()); 
}
