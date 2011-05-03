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

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/model.hh>
#include <mia/3d/timestep.hh>
#include <mia/3d/deformer.hh>

#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
using namespace boost::unit_test;

namespace bfs=::boost::filesystem; 


NS_MIA_USE
using namespace std; 
using namespace boost; 

template <typename P>
void test_plugin(const P& i)
{
	BOOST_CHECK_MESSAGE(i.second->test(true), i.second->get_long_name()); 

}

static void prepare_regplugin_path()
{

	std::list< ::boost::filesystem::path> searchpath; 
	::boost::filesystem::path::default_name_check(::boost::filesystem::portable_posix_name); 
	searchpath.push_back( ::boost::filesystem::path("reg3d")); 
	
	C3DRegModelPluginHandler::set_search_path(searchpath); 
	C3DRegTimeStepPluginHandler::set_search_path(searchpath); 
	
}

void add_regplugin_tests(test_suite* test)
{	
	prepare_regplugin_path(); 
	test->add( BOOST_PARAM_TEST_CASE( &test_plugin<C3DRegModelPluginHandler::value_type>, 
					  C3DRegModelPluginHandler::instance().begin(),
					  C3DRegModelPluginHandler::instance().end() 
			   ));
	
	test->add( BOOST_PARAM_TEST_CASE( &test_plugin<C3DRegTimeStepPluginHandler::value_type>, 
					  C3DRegTimeStepPluginHandler::instance().begin(),
					  C3DRegTimeStepPluginHandler::instance().end() 
			   ));
}
