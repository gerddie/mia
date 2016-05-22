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


#include <mia/internal/autotest.hh>
#include <mia/core/filetools.hh>

#include <stdexcept>
#include <climits>


#ifndef MAX_PATH
#define MAX_PATH 4096
#endif


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_CASE( test_get_filename_number_pattern_width )
{
	BOOST_CHECK_EQUAL(get_filename_number_pattern_width("bla/bulb0000.png"), 4u);
	BOOST_CHECK_EQUAL(get_filename_number_pattern_width("bla/bulb000.png"), 3u);
	BOOST_CHECK_EQUAL(get_filename_number_pattern_width("bla/bulb0000.v.gz"), 0u);
	BOOST_CHECK_EQUAL(get_filename_number_pattern_width("bla/bulb0000s.gz"), 0u);
}

struct Fixture_filename_number_pattern {
	void run(const string& name, const string& expect_base, const string& expect_suffix,
		 size_t expect_width) const;
};

BOOST_FIXTURE_TEST_CASE( test_get_filename_number_pattern, Fixture_filename_number_pattern )
{
	run("data0000.png", "data", ".png", 4);
	run("ldata00000.v", "ldata", ".v", 5);
	run("ldata.v", "ldata", ".v", 0);
	run("ldata0.", "ldata", ".", 1);
	run("ldata0", "ldata", "", 1);

}

void Fixture_filename_number_pattern::run(const string& name, const string& expect_base,
					  const string& expect_suffix, size_t expect_width) const
{
	string base;
	string suffix;
	size_t nwidth;
	nwidth = get_filename_number_pattern(name, base, suffix);
	BOOST_CHECK_EQUAL(nwidth, expect_width);
	BOOST_CHECK_EQUAL(base, expect_base);
	BOOST_CHECK_EQUAL(suffix, expect_suffix);
}


struct Fixture_filename_split_pattern {
	void run(const string& name, const string& expect_base, const string& expect_suffix,
		 const string& expect_number) const;
};

BOOST_FIXTURE_TEST_CASE( test_split_filename_number_pattern, Fixture_filename_split_pattern )
{
	run("data0000.png", "data", ".png", "0000");
	run("ldata00000.v", "ldata", ".v", "00000");
	run("ldata.v", "ldata", ".v", "");
	run("ldata0.", "ldata", ".", "0");
	run("ldata0", "ldata", "", "0");

}

BOOST_AUTO_TEST_CASE( test_get_consecutive_numbered_files )
{
	string filename(MIA_SOURCE_ROOT"/testdata/IM-0001-0000.dcm");
	size_t minrange = 0;
	size_t maxrange = 0;
	size_t format_width = 0; 
	
	auto result = get_filename_pattern_and_range(filename, minrange, maxrange, format_width);

	BOOST_CHECK_EQUAL(minrange, 1u); 
	BOOST_CHECK_EQUAL(maxrange, 18u); 
	BOOST_CHECK_EQUAL(format_width, 4u);
	BOOST_CHECK_EQUAL(result, string(MIA_SOURCE_ROOT"/testdata/IM-0001-%04d.dcm")); 
}

BOOST_AUTO_TEST_CASE( test_get_filename_pattern_and_range )
{
	string filename(MIA_SOURCE_ROOT"/testdata/IM-0001-0000.dcm");

	auto filenames = get_consecutive_numbered_files(filename);

	BOOST_CHECK_EQUAL(filenames.size(), 17u);
	for (unsigned i = 1; i < filenames.size(); ++i) {
		char buffer[MAX_PATH +1]; 
		snprintf(buffer, MAX_PATH, MIA_SOURCE_ROOT"/testdata/IM-0001-%04d.dcm", i);
		BOOST_CHECK_EQUAL(filenames[i-1], string(buffer));
	}
}



void Fixture_filename_split_pattern::run(const string& name, const string& expect_base,
					  const string& expect_suffix, const string& expect_number) const
{
	string base;
	string suffix;
	string numbers;
	split_filename_number_pattern(name, base, suffix, numbers);
	BOOST_CHECK_EQUAL(numbers, expect_number);
	BOOST_CHECK_EQUAL(base, expect_base);
	BOOST_CHECK_EQUAL(suffix, expect_suffix);
}
