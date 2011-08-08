/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/probmap.hh>

using namespace std;
NS_MIA_USE;

BOOST_AUTO_TEST_CASE( test_probmap_store_load )
{
	CProbabilityVector pv(3, 10);

	for (size_t i = 0; i < 10; ++i) {
		for (size_t k = 0; k < 3; ++k) {
			pv[k][i] = i + k ;
		}
	}

	BOOST_REQUIRE(pv.save("test.pv"));

	CProbabilityVector pv_test("test.pv");

	BOOST_CHECK(pv == pv_test);
	unlink("test.pv");

}

BOOST_AUTO_TEST_CASE( test_labelmap_store_load )
{
	CLabelMap a;

	a[1] = 2;
	a[2] = 3;
	a[3] = 2;

	BOOST_REQUIRE(a.save("test.lm"));

	CLabelMap t("test.lm");
	BOOST_CHECK(a == t);

	t[2] = 4;
	BOOST_CHECK(!(a == t));

	CLabelMap b;
	BOOST_CHECK(!(a == b));
	unlink("test.lm");
}
