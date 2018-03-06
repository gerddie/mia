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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/core/utils.hh>

#include <cmath>

NS_MIA_USE
using std::numeric_limits;


BOOST_AUTO_TEST_CASE(test_sincos)
{
#ifndef _GNU_SOURCE
       double x = M_PI / 3.0;
       double s;
       double c;
       sincos(x, &s, &c);
       BOOST_CHECK_CLOSE(c, 0.5, 0.01);
       BOOST_CHECK_CLOSE(s, sqrt(3.0) / 2.0, 0.01);
       double fx = M_PI / 3.0;
       double fs;
       double fc;
       sincos(fx, &fs, &fc);
       BOOST_CHECK_CLOSE(fc, 0.5f, 0.01f);
       BOOST_CHECK_CLOSE(fs, sqrtf(3.0f) / 2.0f, 0.01f);
#endif
}


BOOST_AUTO_TEST_CASE( round_clamped )
{
       double xmax = numeric_limits<double>::max();
       BOOST_CHECK_EQUAL(mia_round_clamped<float>(xmax), numeric_limits<float>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<int>(xmax), numeric_limits<int>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<short>(xmax), numeric_limits<short>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<signed char>(xmax), numeric_limits<signed char>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned int>(xmax), numeric_limits<unsigned int>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned short>(xmax), numeric_limits<unsigned short>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned char>(xmax), numeric_limits<unsigned char>::max());
       double xmin = -numeric_limits<double>::max();
       BOOST_CHECK_EQUAL(mia_round_clamped<float>(xmin), -numeric_limits<float>::max());
       BOOST_CHECK_EQUAL(mia_round_clamped<int>(xmin), numeric_limits<int>::min());
       BOOST_CHECK_EQUAL(mia_round_clamped<short>(xmin), numeric_limits<short>::min());
       BOOST_CHECK_EQUAL(mia_round_clamped<signed char>(xmin), numeric_limits<signed char>::min());
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned int>(xmin), 0u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned short>(xmin), 0u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned char>(xmin), 0u);
       double xp = 4.567;
       BOOST_CHECK_EQUAL(mia_round_clamped<float>(xp), 4.567f);
       BOOST_CHECK_EQUAL(mia_round_clamped<int>(xp), 5);
       BOOST_CHECK_EQUAL(mia_round_clamped<short>(xp), 5);
       BOOST_CHECK_EQUAL(mia_round_clamped<signed char>(xp), 5);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned int>(xp), 5u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned short>(xp), 5u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned char>(xp), 5u);
       double xm = -3.3;
       BOOST_CHECK_EQUAL(mia_round_clamped<float>(xm), -3.3f);
       BOOST_CHECK_EQUAL(mia_round_clamped<int>(xm), -3);
       BOOST_CHECK_EQUAL(mia_round_clamped<short>(xm), -3);
       BOOST_CHECK_EQUAL(mia_round_clamped<signed char>(xm), -3);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned int>(xm), 0u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned short>(xm), 0u);
       BOOST_CHECK_EQUAL(mia_round_clamped<unsigned char>(xm), 0u);
};
