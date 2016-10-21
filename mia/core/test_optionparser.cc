/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/optionparser.hh>

NS_MIA_USE
using namespace std;


string parts2string(CComplexOptionParser::const_iterator b, CComplexOptionParser::const_iterator e)
{
	string result("");

	for (auto i = b; i != e; ++i){
		if (i != b)
			result.append("+");
		result.append(i->first);
		result.append(":");
		for (auto j = i->second.begin();
		     j != i->second.end(); ++j) {
			if (j != i->second.begin())
				result.append(",");

			result.append(j->first);
			result.append("=");
			result.append(j->second);
		}
	}
	return result;
}

BOOST_AUTO_TEST_CASE( test_parsing_a )
{
	const char *paramstr="ssd";
	CComplexOptionParser scanner(paramstr);
	BOOST_REQUIRE(scanner.size() == 1);

	const CComplexOptionParser::const_iterator part = scanner.begin();

	BOOST_CHECK(part->first == "ssd");
	BOOST_CHECK(part->second.size() == 0);
}

BOOST_AUTO_TEST_CASE( test_parsing)
{

	CComplexOptionParser::CParts parts;
	CParsedOptions option;

	option["x1"] = string("value1");
	option["x2"] = string("value2");

	parts.push_back(make_pair("alpha", option));

	option.clear();

	option["y1"] = string("v1");
	option["y2"] = string("v2");
	option["x3"] = string("v3");
	option["x4"] = string("v4");

	parts.push_back(make_pair("beta", option));

	option.clear();

	option["z1"] = string("value1");
	option["z2"] = string("v2");
	option["z3"] = string("value3");
	option["z4"] = string("v4");

	parts.push_back(make_pair("alpha", option));

	option.clear();
	parts.push_back(make_pair("gamma", option));

	string param = parts2string(parts.begin(), parts.end());
	CComplexOptionParser scanner(param);
	string param2 = parts2string(scanner.begin(), scanner.end());

	BOOST_CHECK (param == param2);
}


BOOST_AUTO_TEST_CASE( test_parsing_two_layer )
{
	const char *paramstr="ssd:a=[b:c=[e:d=0]]";
	CComplexOptionParser scanner(paramstr);
	BOOST_REQUIRE(scanner.size() == 1);

	const auto part = scanner.begin();

	BOOST_CHECK(part->first == "ssd");
	BOOST_CHECK(part->second.size() == 1);
	auto a = part->second.find("a"); 
	BOOST_CHECK( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "b:c=[e:d=0]"); 
}

BOOST_AUTO_TEST_CASE( test_parsing_exponents )
{
	const char *paramstr="ssd:c=[1e+6]";
	CComplexOptionParser scanner(paramstr);
	BOOST_REQUIRE(scanner.size() == 1);

	const auto part = scanner.begin();

	BOOST_CHECK(part->first == "ssd");
	BOOST_CHECK(part->second.size() == 1);
	auto a = part->second.find("c"); 
	BOOST_CHECK( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "1e+6"); 
}

BOOST_AUTO_TEST_CASE( test_parsing_chain )
{
	const char *paramstr="ssd:c=[1e+6]+mi:b=12,c=3";
	CComplexOptionParser scanner(paramstr);
	BOOST_REQUIRE(scanner.size() == 2);

	auto part = scanner.begin();

	BOOST_CHECK_EQUAL(part->first, "ssd");
	BOOST_CHECK_EQUAL(part->second.size(), 1);
	auto a = part->second.find("c"); 
	BOOST_CHECK( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "1e+6"); 

	++part; 

	BOOST_CHECK_EQUAL(part->first, "mi");
	BOOST_CHECK_EQUAL(part->second.size(), 2);
	a = part->second.find("c"); 
	BOOST_REQUIRE( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "3"); 


	a = part->second.find("b"); 
	BOOST_REQUIRE( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "12"); 
	
}
