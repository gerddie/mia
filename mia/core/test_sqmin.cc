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

#include <climits>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/sqmin.hh>

NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_sqmin )
{
       BOOST_CHECK_CLOSE(min_ax2_bx_c(0.5, 2.0, 1.0, 1.0), 0.75, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(2.0, 2.0, 1.0, 1.0), 1.5, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(2.0, 2.0, 1.0, 2.0), 1.0, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(0.5, 2.0, 0.0, 1.0), 1.0, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(0.5, 0.0, 2.0, 1.0), 0.0, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(2.0, 1.0, 2.0, 4.0), 0.0, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(2.0, 0.0, 1.0, 2.0), 0.0, 0.0001);
       BOOST_CHECK_CLOSE(min_ax2_bx_c(2.0, 2.0, 1.0, 0.0), 2.0, 0.0001);
}
