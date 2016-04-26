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
	pm["min"] =   CParamList::PParameter(make_ci_param(read.min, -100, 20, true, "a float value"));
	pm["max"] =   CParamList::PParameter(make_ci_param(read.max, -10, 200, true, "a float value"));
	pm["nana"] =  CParamList::PParameter(new CStringParameter(read.nana, CCmdOptionFlags::required,
								  "a string value"));
	pm["n"] =     CParamList::PParameter(make_ci_param(read.n, -200, 100, true, "an int value"));

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

void try_parsing_and_setting(const char *options, const params& expect, params read)
{
	CParamList pm;
	pm["kill"] =  CParamList::PParameter(new CBoolParameter(read.kill, false, "a bool value"));
	pm["min"] =   CParamList::PParameter(make_ci_param(read.min, -100, 20, false, "a float value"));
	pm["max"] =   CParamList::PParameter(make_ci_param(read.max, -10, 200, false, "a float value"));
	pm["nana"] =  CParamList::PParameter(new CStringParameter(read.nana, CCmdOptionFlags::none, "a string value"));
	pm["n"] =     CParamList::PParameter(make_ci_param(read.n, -200, 100, false, "an int value"));

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
	
	char option_success[] = "plugin:kill=1,min=1.3,nana=name,max=7.1,no=5";
	
	BOOST_CHECK_THROW(try_parsing_and_setting(option_success, p_expect), 
			  invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_fail2)
{
	params p_expect = { true, 5, 1.3, 7.1, "huhu" };
	
	char option_success[] = "plugin:min=1.3,nana=name,max=7.1";
	BOOST_CHECK_THROW(try_parsing_and_setting(option_success, p_expect), invalid_argument); 
}


BOOST_AUTO_TEST_CASE( test_reset )
{
	params read = { false, 0, 0.0, 0.0, "pong" };
	params p_expect_keep = { false, 0, 0.0, 0.0, "pong" };
	params p_expect = { false, 1, 1.1, 3.1, "ping" };
	try {

		CParamList pm;
		pm["kill"] =  CParamList::PParameter(new CBoolParameter(read.kill, false, "a bool value"));
		pm["min"] =   CParamList::PParameter(make_ci_param(read.min, -100, 20, false, "a float value"));
		pm["max"] =   CParamList::PParameter(make_ci_param(read.max, -10, 200, false, "a float value"));
		pm["nana"] =  CParamList::PParameter(new CStringParameter(read.nana, CCmdOptionFlags::none, "a string value"));
		pm["n"] =     CParamList::PParameter(make_ci_param(read.n, -200, 100, false, "an int value"));
		
		CComplexOptionParser cpo("plugin:kill=0,min=1.1,nana=ping,max=3.1,n=1");
		BOOST_CHECK(cpo.size() == 1);
		const CParsedOptions& opts = cpo.begin()->second;
		
		pm.set(opts);
		
		BOOST_CHECK_EQUAL(p_expect.kill, read.kill);
		BOOST_CHECK_EQUAL(p_expect.n, read.n);
		BOOST_CHECK_EQUAL(p_expect.min, read.min);
		BOOST_CHECK_EQUAL(p_expect.max, read.max);
		BOOST_CHECK_EQUAL(p_expect.nana, read.nana);
		
		CComplexOptionParser cpo2("plugin");
		pm.set(cpo2.begin()->second);
		
		BOOST_CHECK_EQUAL(p_expect_keep.kill, read.kill);
		BOOST_CHECK_EQUAL(p_expect_keep.n, read.n);
		BOOST_CHECK_EQUAL(p_expect_keep.min, read.min);
		BOOST_CHECK_EQUAL(p_expect_keep.max, read.max);
		BOOST_CHECK_EQUAL(p_expect_keep.nana, read.nana);

	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}
}

