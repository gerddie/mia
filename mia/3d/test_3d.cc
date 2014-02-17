/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/core/cmdlineparser.hh>

using namespace mia;
using namespace boost::unit_test;


extern void add_3dvector_tests(test_suite* suite);
extern void add_3ddatafield_tests(test_suite* suite);
extern void add_3dimage_tests(test_suite* suite);
extern void add_3dinterpol_tests(test_suite* suite);

const SProgramDescription g_general_help = {
        {pdi_group, "Tests"}, 
	{pdi_short, "3D tests"}, 
	{pdi_description, "Run various test for 3D data."}
}; 

bool init_unit_test_suite( )
{

	test_suite *suite = &framework::master_test_suite();

	add_3dvector_tests(suite);
	add_3ddatafield_tests(suite);
	add_3dimage_tests(suite);
	add_3dinterpol_tests(suite);
	return true;
}

int BOOST_TEST_CALL_DECL
do_main( int argc, char* argv[] )
{
	if (CCmdOptionList(g_general_help).parse(argc, (const char**)argv, "boost-test-option") != CCmdOptionList::hr_no) 
		return 0; 
	return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main);
