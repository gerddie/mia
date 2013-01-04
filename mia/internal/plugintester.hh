/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <climits>

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <miaconfig.h>
#include <mia/core/factory.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_BEGIN 

template <typename Plugin> 
typename Plugin::UniqueProduct BOOST_TEST_create_from_plugin(const char *param) 
{
	assert(param); 
	Plugin plugin; 
	CComplexOptionParser param_list(param);
	BOOST_REQUIRE(param_list.size() > 0); 
	BOOST_CHECK_EQUAL(plugin.get_name(), param_list.begin()->first); 
	return typename Plugin::UniqueProduct(plugin.create(param_list.begin()->second, param)); 
}


NS_MIA_END 


const mia::SProgramDescription description = {
	{mia::pdi_group, "Test"}, 
	{mia::pdi_short, "Unit test for a plug-in."}, 
	{mia::pdi_description,"This program runs a set of tests."}, 
	{mia::pdi_example_descr, "Example text"}, 
	{mia::pdi_example_code, ""}
};

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	setenv("MIA_PLUGIN_TESTPATH", MIA_BUILD_ROOT "/plugintest/" PLUGIN_INSTALL_PATH,1);
#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
	if (mia::CCmdOptionList(description).parse(argc, argv, "boost-test-option") != mia::CCmdOptionList::hr_no) 
		return 0; 
	mia::cvdebug() << "Initialize test ...\n"; 
	return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
