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

#include <climits>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>


using namespace mia; 
using namespace boost::unit_test;

namespace bfs = ::boost::filesystem; 

extern void add_2ddatafield_tests(test_suite* suite); 
extern void add_2dimage_tests(test_suite* suite); 
extern void add_2dinterpol_tests(test_suite* suite); 
extern void add_2dimage_nfg_tests(test_suite* suite);

bool init_unit_test_suite( ) 
{
	if (bfs::path::default_name_check_writable())
		bfs::path::default_name_check(::boost::filesystem::portable_posix_name); 


	
	test_suite *suite = &framework::master_test_suite(); 

	add_2ddatafield_tests(suite); 
	add_2dimage_tests(suite); 
	add_2dinterpol_tests(suite); 
	add_2dimage_nfg_tests(suite); 


	return true;
}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	CCmdOptionList().parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}
