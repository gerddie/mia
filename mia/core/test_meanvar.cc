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

#include <vector>

#include <mia/internal/autotest.hh>
#include <mia/core/meanvar.hh>

NS_MIA_USE;
using namespace std;

BOOST_AUTO_TEST_CASE( test_meanvar_0 )
{
       vector<double> data(0);
       auto r = mean_var(data.begin(), data.end());
       BOOST_CHECK_EQUAL(r.first, 0.0);
       BOOST_CHECK_EQUAL(r.second, 0.0);
}

BOOST_AUTO_TEST_CASE( test_meanvar_1 )
{
       vector<double> data(1, 1.0);
       auto r = mean_var(data.begin(), data.end());
       BOOST_CHECK_EQUAL(r.first, 1.0);
       BOOST_CHECK_EQUAL(r.second, 0.0);
}

BOOST_AUTO_TEST_CASE( test_meanvar_3 )
{
       vector<double> data(3);
       data[0] = 1.0;
       data[1] = 3.0;
       data[2] = 5.0;
       auto r = mean_var(data.begin(), data.end());
       BOOST_CHECK_EQUAL(r.first, 3.0);
       BOOST_CHECK_EQUAL(r.second, 2.0);
}

