/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/core/fullstats.hh>

NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_stats1 )
{
	const float test_data[5] = {
		1.0, 0.0, -1.0, 2.0, 3.0
	};

	CFullStats stats(test_data, test_data + 5);
	BOOST_CHECK_CLOSE(stats.mean(), 1.0, 0.1);
	BOOST_CHECK_CLOSE(stats.sigma(), 1.5811, 0.1);
	BOOST_CHECK_EQUAL(stats.median(), 1.0);
	BOOST_CHECK_EQUAL(stats.max(), 3.0);
	BOOST_CHECK_EQUAL(stats.min(), -1.0);
}

BOOST_AUTO_TEST_CASE( test_stats2 )
{
	const float test_data[4] = {
		1.0, 0.0, -1.0, 2.0
	};
	CFullStats stats(test_data, test_data + 4);
	BOOST_CHECK_EQUAL(stats.median(), 0.5);
}

