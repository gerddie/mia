/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * Gert Wollny <gert.wollny at web.de>
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


#define BOOST_TEST_MODULE 3DIMAGEIO
#include <mia/internal/autotest.hh>
#include <algorithm>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dimageiotest.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_USE
using namespace boost;
using namespace std;
using namespace boost::unit_test;
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE(test_3dimageio_plugin_avail)
{
	const C3DImageIOPluginHandler::Instance&  handler = C3DImageIOPluginHandler::instance();

	BOOST_CHECK(handler.size() == 5);
	BOOST_CHECK(handler.get_plugin_names() == "analyze datapool inria vff vtk ");

}

bool init_unit_test_suite( )
{
	cvdebug() << "init\n";
	init_unit_test();
	add_3dimageio_plugin_tests( &framework::master_test_suite());
	return true;
}
