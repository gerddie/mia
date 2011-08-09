/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/core/filetools.hh>


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
