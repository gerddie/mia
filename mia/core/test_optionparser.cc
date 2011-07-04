/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

	for (CComplexOptionParser::const_iterator i = b; i != e; ++i){
		if (i != b)
			result.append("+");
		result.append(i->first);
		result.append(":");
		for (CParsedOptions::const_iterator j = i->second.begin();
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
	typedef CComplexOptionParser::CParts::value_type CValue;

	option["x1"] = string("value1");
	option["x2"] = string("value2");

	parts.insert(CValue("alpha", option));

	option.clear();

	option["y1"] = string("v1");
	option["y2"] = string("v2");
	option["x3"] = string("v3");
	option["x4"] = string("v4");

	parts.insert(CValue("beta", option));

	option.clear();

	option["z1"] = string("value1");
	option["z2"] = string("v2");
	option["z3"] = string("value3");
	option["z4"] = string("v4");

	parts.insert(CValue("alpha", option));

	option.clear();
	parts.insert(CValue("gamma", option));

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

	const CComplexOptionParser::const_iterator part = scanner.begin();

	BOOST_CHECK(part->first == "ssd");
	BOOST_CHECK(part->second.size() == 1);
	auto a = part->second.find("a"); 
	BOOST_CHECK( a != part->second.end()); 
	BOOST_CHECK_EQUAL(a->second, "b:c=[e:d=0]"); 
}
