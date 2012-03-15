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


#define BOOST_TEST_MODULE 3DIMAGEIO



#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_ALTERNATIVE_INIT_API
#define BOOST_TEST_MAIN
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <algorithm>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dimageiotest.hh>

NS_MIA_USE
using namespace boost;
using namespace std;
using namespace boost::unit_test;
namespace bfs=::boost::filesystem;

const SProgramDescription g_general_help = {
	"Tests", 
	"test 3D image io", 
	"Run various test for 3D image IO.", 
	NULL, 
	NULL
}; 


BOOST_AUTO_TEST_CASE(test_3dimageio_plugin_avail)
{
	const C3DImageIOPluginHandler::Instance&  handler = C3DImageIOPluginHandler::instance();

	BOOST_CHECK(handler.size() == 5);
	BOOST_CHECK(handler.get_plugin_names() == "analyze datapool inria vff vtk ");

}


C3DImageIOPluginHandlerTestPath init_3dimage_path; 

bool init_unit_test_suite( )
{

	cvdebug() << "init\n";
	init_unit_test();
	add_3dimageio_plugin_tests( &framework::master_test_suite());
	return true;
}



NS_MIA_USE; 
int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{

#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
	if (CCmdOptionList(g_general_help).parse(argc, (const char **)argv) != CCmdOptionList::hr_no) 
		return 0; 
	cvdebug() << "Initialize test ...\n"; 
	return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}

