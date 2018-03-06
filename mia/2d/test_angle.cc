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
#include <cmath>

#include <mia/2d/angle.hh>
using namespace mia;

BOOST_AUTO_TEST_CASE( test_some_angles )
{
       C2DFVector s(0.0, 0.0);
       C2DFVector a(1.0, 0.0);

       for (size_t i = 0; i < 20; ++i) {
              const double ang = i * 2 * M_PI / 180.0;
              C2DFVector b(cos(ang), sin(ang));
              BOOST_CHECK_CLOSE(angle(a, b, s), ang, 0.1);
       }
}

BOOST_AUTO_TEST_CASE( test_some_other_angles )
{
       C2DFVector s(1.0, 1.0);
       C2DFVector a(2.0, 1.0);

       for (size_t i = 0; i < 20; ++i) {
              const double ang = i * 2 * M_PI / 180.0;
              C2DFVector b(cos(ang) + s.x, sin(ang) + s.y) ;
              BOOST_CHECK_CLOSE(angle(a, b, s), ang, 0.1);
       }
}

BOOST_AUTO_TEST_CASE( test_sero_length )
{
       C2DFVector s(1.0, 1.0);
       C2DFVector a(2.0, 1.0);
       C2DFVector b(2.0, 2.0);
       BOOST_CHECK_EQUAL(angle(a, b, a), 0.0);
       BOOST_CHECK_EQUAL(angle(a, b, b), 0.0);
}



