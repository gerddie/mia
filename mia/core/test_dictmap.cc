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

#define BOOST_TEST_DYN_LINK
#include <cassert>
#include <iostream>
#include <cmath>
#include <climits>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/dictmap.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

using namespace std;
using namespace mia;

enum ETest {te_unknown, te_a, te_b, te_c};

BOOST_AUTO_TEST_CASE( test_dictmap )
{

	static const TDictMap<ETest>::Table table[] = {
		{"a", te_a, "test a"},
		{"b", te_b, "test b"},
		{"c", te_c, "test c"},
		{NULL, te_unknown, ""}
	};
	const TDictMap<ETest> map(table);

	BOOST_CHECK_EQUAL(map.get_value("a"), te_a);
	BOOST_CHECK_EQUAL(map.get_value("b"), te_b);
	BOOST_CHECK_EQUAL(map.get_value("c"), te_c);

	set<string> name_set = map.get_name_set();
	BOOST_CHECK_EQUAL(name_set.size(), 3u);

	BOOST_CHECK_EQUAL(string(map.get_name(te_a)), string("a"));
	BOOST_CHECK_EQUAL(string("b"), string(map.get_name(te_b)));
	BOOST_CHECK_EQUAL(string("c"), string(map.get_name(te_c)));

	try {
		map.get_value("d");
		BOOST_FAIL("map.get_value(\"d\") should have thrown an exception");
	}
	catch(invalid_argument& x)  {
		cvdebug() << "expected exception: '"<< x.what() << "'\n";
	}
	catch(...)  {
		BOOST_FAIL("map.get_value(\"d\") should have thrown 'invalid_argument'");
	}
}

BOOST_AUTO_TEST_CASE( test_kill_double_entries )
{


	static const TDictMap<ETest>::Table table[] = {
		{"a", te_a, "test A"},
		{"b", te_b, "test B"},
		{"c", te_c, "test C"},
		{"c", te_b, "test C2"},
		{NULL, te_unknown, ""}
	};

	try {
		const TDictMap<ETest> map(table);
		BOOST_FAIL("'test_kill_double_entries' should have thrown an exception");
	}
	catch(invalid_argument& x)  {
		cvdebug() << "expected exception: '"<< x.what() << "'\n";
	}
	catch(...)  {
		BOOST_FAIL("'test_kill_double_entries' should have thrown 'invalid_argument'");
	}
}

BOOST_AUTO_TEST_CASE( test_default_entry )
{

	static const TDictMap<ETest>::Table table[] = {
		{"a", te_a, "test A"},
		{"b", te_b, "test B"},
		{"c", te_c, "test C"},
		{NULL, te_unknown, ""}
	};
	const TDictMap<ETest> map(table, true);

	BOOST_CHECK_EQUAL(map.get_value("d"), te_unknown);
}
