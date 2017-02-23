/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/core/tools.hh>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
using namespace boost;



BOOST_AUTO_TEST_CASE( test_mia_tools )
{
	BOOST_CHECK_EQUAL(to_string<int>(10), string("10"));
}

BOOST_AUTO_TEST_CASE( test_mia_tools_new )
{
	int test = 0; 
	BOOST_CHECK(from_string<int>("10", test)); 
	BOOST_CHECK_EQUAL(test, 10);

	BOOST_CHECK(from_string(string("11"), test));
	BOOST_CHECK_EQUAL(test, 11);
	
	
	BOOST_CHECK_EQUAL(to_string<int>(10), string("10"));

	BOOST_CHECK(!from_string(string("a11a"), test));
	BOOST_CHECK(!from_string("a11a", test));
	
	BOOST_CHECK(!from_string(string("11_"), test));
	BOOST_CHECK(!from_string("11.", test));

	BOOST_CHECK(from_string("12 ", test));
	BOOST_CHECK_EQUAL(test, 12);

	BOOST_CHECK(from_string("\t13 ", test));
	BOOST_CHECK_EQUAL(test, 13);
}


