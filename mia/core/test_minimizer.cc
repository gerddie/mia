/* -*- mona-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define BOOST_TEST_MODULE MINIMIZER_TEST

#include <mia/internal/autotest.hh>

#include <mia/core/minimizer.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>


using namespace std; 
using namespace mia; 
using namespace boost;
namespace bfs=::boost::filesystem; 


BOOST_AUTO_TEST_CASE( test_load_minimizer_plugins )	
{
	list< bfs::path> searchpath; 

	searchpath.push_back( bfs::path("minimizer"));
	
	CMinimizerPluginPluginHandler::set_search_path(searchpath); 

	const CMinimizerPluginPluginHandler::Instance& handler = CMinimizerPluginPluginHandler::instance();

	BOOST_CHECK_EQUAL(handler.size(), 1u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "gsl "); 

}

