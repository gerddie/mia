/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/model.hh>
#include <mia/2d/timestep.hh>
#include <mia/2d/deformer.hh>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

NS_MIA_USE
using namespace std; 
using namespace boost; 
using namespace boost::unit_test;

namespace bfs=::boost::filesystem; 

template <typename P>
void test_plugins(const P& ph)
{
	for (typename P::const_iterator i = ph.begin(); 
	     ph.end() != i; ++i) {
		cvmsg() << "Testing: " << i->second->get_long_name() << "\n"; 
		if (!i->second->test(true))
			BOOST_FAIL(i->second->get_long_name()); 
	}
}

static void test_regplugin()
{
	test_plugins(C2DRegModelPluginHandler::instance()); 
	test_plugins(C2DRegTimeStepPluginHandler::instance()); 
}

static void prepare_regplugin_path()
{

	std::list< bfs::path> searchpath; 
	bfs::path::default_name_check(bfs::portable_posix_name); 
	searchpath.push_back( bfs::path("reg2d")); 
	
	C2DRegModelPluginHandler::set_search_path(searchpath); 
	C2DRegTimeStepPluginHandler::set_search_path(searchpath); 
	
}


void add_regplugin_tests(test_suite* test)
{	
	prepare_regplugin_path(); 
	test->add( BOOST_TEST_CASE( &test_regplugin));
}

