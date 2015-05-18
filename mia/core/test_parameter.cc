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

#define BOOST_TEST_DYN_LINK
#include <cassert>
#include <iostream>
#include <cmath>
#include <climits>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/parameter.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/parameter.cxx>

NS_MIA_USE

using namespace std;
BOOST_AUTO_TEST_CASE( test_params )
{

	int int_val = 1;
	float float_val = 2.1;
	bool bool_val = false;
	string string_val("initial");
	string sval("replaced");

	CStringParameter param_string(string_val, CCmdOptionFlags::required, "a string value");
	CBoolParameter   param_bool(bool_val, false, "a bool value");

	BOOST_CHECK(!param_bool.required_set());

	param_string.set(sval);
	param_bool.set("1");
	BOOST_CHECK(!param_string.required_set());



	BOOST_CHECK(string_val == sval);
	BOOST_CHECK(bool_val);

}

enum ETest {te_unknown, te_a, te_b, te_c};

BOOST_AUTO_TEST_CASE( test_dict_params)
{

	static const TDictMap<ETest>::Table table[] = {
		{"a", te_a, "test a"},
		{"b", te_b, "test b"},
		{"c", te_c, "test c"},
		{NULL, te_unknown, ""}
	};

	const TDictMap<ETest> map(table, true);
	ETest test1 = te_a;

	CDictParameter<ETest> testp1(test1, map, "some dictionary parameter");

	BOOST_CHECK_EQUAL(test1, te_a);

	BOOST_CHECK(testp1.set("b"));
	BOOST_CHECK_EQUAL(test1, te_b);

	BOOST_CHECK(testp1.set("c"));
	BOOST_CHECK_EQUAL(test1, te_c);
	BOOST_CHECK(testp1.set("chajh"));
	BOOST_CHECK_EQUAL(test1, te_unknown);

	const TDictMap<ETest> map2(table);
	CDictParameter<ETest> testp2(test1, map2, "some dictionary parameter");
	BOOST_CHECK_THROW(testp2.set("chajh"), std::invalid_argument);


}


BOOST_AUTO_TEST_CASE( test_set_params)
{
	set<int> valids;
	valids.insert(1);
	valids.insert(2);

	int val = 1;

	CSetParameter<int> testp1(val, valids, "some dictionary parameter");

	BOOST_CHECK_EQUAL(val, 1);

	BOOST_CHECK(testp1.set("2"));
	BOOST_CHECK_EQUAL(val, 2);

	BOOST_CHECK_THROW(testp1.set("3"), std::invalid_argument);

	BOOST_CHECK_THROW(testp1.set("gjhgjhdf"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_vector_param)
{
	vector<int> val; 
	CTParameter<vector<int>> testp1(val, false, "some vector valued parameter");
	
	BOOST_CHECK(testp1.set("2,3,1,2"));
	BOOST_CHECK_EQUAL(val.size(), 4u);

	BOOST_CHECK_EQUAL(val[0], 2); 
	BOOST_CHECK_EQUAL(val[1], 3); 
	BOOST_CHECK_EQUAL(val[2], 1); 
	BOOST_CHECK_EQUAL(val[3], 2); 
}


BOOST_AUTO_TEST_CASE( test_bounded_param_float_closed)
{
	float v = 0.0; 
	CFBoundedParameter testv(v, EParameterBounds::bf_closed_interval, 
				 {2.0, 4.0}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("2.5"));
	BOOST_CHECK_EQUAL(v, 2.5);
	
	BOOST_CHECK(testv.set("4.0"));
	BOOST_CHECK_EQUAL(v, 4.0);

	BOOST_CHECK(testv.set("2.0"));
	BOOST_CHECK_EQUAL(v, 2.0);
	

	BOOST_CHECK_THROW(testv.set("1.0"), std::invalid_argument);
	BOOST_CHECK_THROW(testv.set("5.0"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_bounded_param_float_open)
{
	float v = 0.0; 
	CFBoundedParameter testv(v, EParameterBounds::bf_open_interval, 
				 {2.0, 4.0}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("2.5"));
	BOOST_CHECK_EQUAL(v, 2.5);
	
	BOOST_CHECK_THROW(testv.set("4.0"), std::invalid_argument); 
	BOOST_CHECK_THROW(testv.set("2.0"), std::invalid_argument);
	BOOST_CHECK_THROW(testv.set("1.0"), std::invalid_argument);
	BOOST_CHECK_THROW(testv.set("5.0"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_bounded_param_int_min_open)
{
	int v = 0; 
	CSIBoundedParameter testv(v, EParameterBounds::bf_min_open, 
				  {-2}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("-1"));
	BOOST_CHECK_EQUAL(v, -1);
	
	BOOST_CHECK_THROW(testv.set("-2"), std::invalid_argument); 
	BOOST_CHECK_THROW(testv.set("-3"), std::invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_bounded_param_int_min_close)
{
	int v = 0; 
	CSIBoundedParameter testv(v, EParameterBounds::bf_min_closed, 
				  {-2}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("-2"));
	BOOST_CHECK_EQUAL(v, -2);

	BOOST_CHECK(testv.set("2"));
	BOOST_CHECK_EQUAL(v, 2);
	
	BOOST_CHECK_THROW(testv.set("-3"), std::invalid_argument); 
}


BOOST_AUTO_TEST_CASE( test_bounded_param_unsignedshort_min_open)
{
	unsigned short v = 0; 
	CUSBoundedParameter testv(v, EParameterBounds::bf_min_open, 
				  {2}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("3"));
	BOOST_CHECK_EQUAL(v, 3);
	
	BOOST_CHECK_THROW(testv.set("2"), std::invalid_argument); 
	BOOST_CHECK_THROW(testv.set("1"), std::invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_bounded_param_unsignedshort_min_close)
{
	unsigned short v = 0; 
	CUSBoundedParameter testv(v, EParameterBounds::bf_min_closed, 
				  {2}, false, "a bounded test value"); 
	
	
	BOOST_CHECK(testv.set("2"));
	BOOST_CHECK_EQUAL(v, 2);

	BOOST_CHECK(testv.set(" 3"));
	BOOST_CHECK_EQUAL(v, 3);
	
	BOOST_CHECK_THROW(testv.set("1"), std::invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_bounded_param_unsignedshort_min_close_negative_input)
{
	unsigned short v = 0; 
	CUSBoundedParameter testv(v, EParameterBounds::bf_min_closed, 
				  {2}, false, "a bounded test value"); 
	
	BOOST_CHECK_THROW(testv.set(" -1"), std::invalid_argument); 
}




