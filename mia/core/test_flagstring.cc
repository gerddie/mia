/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/internal/autotest.hh>
#include <mia/core/flagstring.hh>

enum ETestFlags {
	tf_none = 0,
	tf_one  = 1,
	tf_two  = 2,
	tf_four = 4,
	tf_cb1  = 9,
};

const CFlagString::Table flag_table[] = {
	{'o', tf_one},
	{'t', tf_two},
	{'h', tf_one |  tf_two},
	{'f', tf_four},
	{'c', tf_cb1 },
	{ 0, tf_none}
};

struct FlagStringFixture {
	FlagStringFixture();

	CFlagString flag_trans;
};


BOOST_FIXTURE_TEST_CASE ( test_flags, FlagStringFixture )
{
	BOOST_CHECK_EQUAL(flag_trans.get("o"), tf_one);
	BOOST_CHECK_EQUAL(flag_trans.get("t"), tf_two);

	BOOST_CHECK(flag_trans.get("h") & tf_two);
	BOOST_CHECK(flag_trans.get("h") & tf_one);
}

BOOST_FIXTURE_TEST_CASE ( test_throw, FlagStringFixture )
{
	BOOST_CHECK_THROW(flag_trans.get("x"), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE ( test_names, FlagStringFixture )
{
	BOOST_CHECK_EQUAL(flag_trans.get_flagnames(), "cfhot");
}


BOOST_FIXTURE_TEST_CASE ( test_backtranslate, FlagStringFixture )
{
	BOOST_CHECK_EQUAL(flag_trans.get(1), "o");
	BOOST_CHECK_EQUAL(flag_trans.get(2), "t");
	BOOST_CHECK_EQUAL(flag_trans.get(3), "h");
	BOOST_CHECK_EQUAL(flag_trans.get(5), "of");

	// combinations may also overlap
	BOOST_CHECK_EQUAL(flag_trans.get(11), "othc");
}



FlagStringFixture::FlagStringFixture():flag_trans(flag_table)
{
}
