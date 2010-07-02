/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#define BOOST_TEST_DYN_LINK
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/optparam.hh>

NS_MIA_USE

using namespace std;


struct params {
	bool kill;
	int n;
	float min;
	float max;
	string nana;
};

void try_parsing_and_setting(const char *options, const params& expect)
{
	params read;

	CParamList pm;
	pm["kill"] =  CParamList::PParameter(new CBoolParameter(read.kill, false, "a bool value"));
	pm["min"] =   CParamList::PParameter(new CFloatParameter(read.min, -100, 20, true, "a float value"));
	pm["max"] =   CParamList::PParameter(new CFloatParameter(read.max, -10, 200, true, "a float value"));
	pm["nana"] =  CParamList::PParameter(new CStringParameter(read.nana, true, "a string value"));
	pm["n"] =     CParamList::PParameter(new CIntParameter(read.n, -200, 100, true, "an int value"));

	CComplexOptionParser cpo(options);
	BOOST_CHECK(cpo.size() == 1);
	const CParsedOptions& opts = cpo.begin()->second;

	pm.set(opts);
	pm.check_required();

	BOOST_CHECK(expect.kill == read.kill);
	BOOST_CHECK(expect.n == read.n);
	BOOST_CHECK(expect.min == read.min);
	BOOST_CHECK(expect.max == read.max);
	BOOST_CHECK(expect.nana == read.nana);
}

BOOST_AUTO_TEST_CASE(test_sucess1)
{
	params p_expect = { true, 5, 1.3, 7.1, "name" };
	try {
		char option_success[] = "plugin:kill=1,min=1.3,nana=name,max=7.1,n=5";
		try_parsing_and_setting(option_success, p_expect);
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}

BOOST_AUTO_TEST_CASE( test_sucess2)
{
	params p_expect = { false, 1, 1.1, 3.1, "ping" };
	try {
		char option_success[] = "plugin:kill=0,min=1.1,nana=ping,max=3.1,n=1";
		try_parsing_and_setting(option_success, p_expect);
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}

BOOST_AUTO_TEST_CASE( test_fail1)
{
	params p_expect = { true, 5, 1.3, 7.1, "huhu" };
	try {
		char option_success[] = "plugin:kill=1,min=1.3,nana=name,max=7.1,no=5";
		try_parsing_and_setting(option_success, p_expect);
		BOOST_FAIL("did not catch the unknown parameter");
	}
	catch (invalid_argument& x) {
		// it should throw ...
		BOOST_MESSAGE(x.what());
	}

}

BOOST_AUTO_TEST_CASE( test_fail2)
{
	params p_expect = { true, 5, 1.3, 7.1, "huhu" };
	try {
		char option_success[] = "plugin:min=1.3,nana=name,max=7.1";
		try_parsing_and_setting(option_success, p_expect);
		BOOST_FAIL("did not catch the missing parameter");
	}
	catch (invalid_argument& x) {
		// it should throw ...
		BOOST_MESSAGE(x.what());
	}
}

