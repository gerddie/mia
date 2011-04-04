/* -*- mia-c++  -*-
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <boost/filesystem/path.hpp>
#include <mia/2d/2dfilter.hh>


NS_MIA_USE
using namespace boost;
using namespace std;
namespace bfs=boost::filesystem; 

BOOST_AUTO_TEST_CASE( test_load_plugins ) 
{	
	list< bfs::path> plugpath;
	plugpath.push_back(bfs::path("combiner"));
	C2DImageCombinerPluginHandler::set_search_path(plugpath);

	const C2DImageCombinerPluginHandler::Instance& handler = C2DImageCombinerPluginHandler::instance(); 
	BOOST_CHECK_EQUAL(handler.size(), 4u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "add div mul sub ");
}
