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

#include <stdexcept>
#include <cmath>
#include <iomanip>

#include <mia/core/property_flags.hh>


using namespace std;
using namespace mia;

BOOST_AUTO_TEST_CASE( test_has_flag )
{
       const char *flag1 = "flag1";
       const char *flag2 = "flag2";
       CPropertyFlagHolder holder;
       holder.add(flag1);
       BOOST_CHECK(holder.has(flag1));
       BOOST_CHECK(!holder.has(flag2));
       holder.add(flag2);
       BOOST_CHECK(holder.has(flag2));
}


BOOST_AUTO_TEST_CASE( test_has_all_flag )
{
       const char *flag1 = "flag1";
       const char *flag2 = "flag2";
       CPropertyFlagHolder holder1;
       holder1.add(flag1);
       holder1.add(flag2);
       CPropertyFlagHolder holder2;
       holder2.add(flag1);
       BOOST_CHECK(holder1.has_all_in(holder2));
       BOOST_CHECK(!holder2.has_all_in(holder1));
}


BOOST_AUTO_TEST_CASE( test_get_missing_flags )
{
       const char *flag1 = "flag1";
       const char *flag2 = "flag2";
       const char *flag3 = "flag3";
       CPropertyFlagHolder holder1;
       holder1.add(flag1);
       CPropertyFlagHolder holder2;
       holder2.add(flag1);
       BOOST_CHECK(holder2.get_missing_properties(holder1).empty());
       holder1.add(flag2);
       holder1.add(flag3);
       auto result = holder2.get_missing_properties(holder1);
       BOOST_CHECK_EQUAL(result.size(), 2u);
       BOOST_CHECK(result.find(flag2) != result.end());
       BOOST_CHECK(result.find(flag3) != result.end());
}
