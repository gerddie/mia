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
#include <mia/2d/filter/admean.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace admean_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_adaptmed )
{
       const size_t size_x = 3;
       const size_t size_y = 3;
       const int width = 1;
       const float src[size_y][size_x] = {
              { 0, 1, 0},
              { 1, 2, 1},
              { 0, 1, 0}
       };
       // "hand filtered" w = 1 -> 3x3
       const float src_ref[size_y][size_x] = {
              { 3.0 / 4.0, 29.0 / 34.0,  3.0 / 4.0},
              {29.0 / 34.0,  2.0 / 3.0, 29.0 / 34.0},
              { 3.0 / 4.0, 29.0 / 34.0,  3.0 / 4.0}
       };
       C2DBounds size ( size_x, size_y );
       C2DFImage *src_img = new C2DFImage ( size );

       for ( size_t y = 0; y < size_y; ++y )
              for ( size_t x = 0; x < size_x; ++x )
                     ( *src_img ) ( x, y ) = src[y][x];

       C2DAdmean admean ( width );
       P2DImage src_wrap ( src_img );
       P2DImage res_wrap = admean.filter( *src_wrap );
       const C2DFImage *res_img = dynamic_cast<const C2DFImage *>(res_wrap.get());
       BOOST_REQUIRE( res_img );
       BOOST_CHECK_EQUAL(res_img->get_size(), src_img->get_size());

       for ( size_t y = 0; y < size_y; ++y )
              for ( size_t x = 0; x < size_x; ++x )
                     BOOST_CHECK_CLOSE ( ( *res_img ) ( x, y ), src_ref[y][x], 0.01 );
}
