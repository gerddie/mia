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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/tmean.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace tmean_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_2dfilter_tmean_float )
{
       const size_t size_x = 7;
       const size_t size_y = 5;
       const int src[size_y][size_x] = {
              { 0, 1, 2, 3, 2, 3, 5},
              { 2, 5, 2, 3, 5, 3, 2},
              { 1, 2, 7, 6, 4, 2, 1},
              { 3, 4, 4, 3, 4, 3, 2},
              { 1, 3, 2, 4, 5, 6, 2}
       };
       // "hand filtered" w = 1 -> 3x3 using zero boundary conditions
       //
       const float src_ref[size_y][size_x] = {
              { 0.0,      0.0,      3.0, 17.0 / 6.0, 19.0 / 6.0, 10.0 / 3.0, 13.0 / 4.0},
              { 3.0, 20.0 / 6.0, 30.0 / 8.0, 34.0 / 9.0, 31.0 / 9.0,  13.0 / 4.0,      3.0},
              { 0.0, 29.0 / 8.0, 4.0 / 1.0, 38.0 / 9.0, 11.0 / 3.0, 25.0 / 8.0,    0},
              { 3.0, 25.0 / 7.0, 35.0 / 9.0, 13.0 / 3.0, 37.0 / 9.0, 28.0 / 8.0,  3.0},
              { 0.0, 16.0 / 5.0, 10.0 / 3.0, 11.0 / 3.0, 25.0 / 6.0, 11.0 / 3.0, 13.0 / 4.0}
       };
       C2DBounds size(size_x, size_y);
       C2DFImage *src_img = new C2DFImage(size);

       for (size_t y = 0; y < size_y; ++y)
              for (size_t x = 0; x < size_x; ++x)
                     (*src_img)(x, y) = src[y][x];

       auto tmean = BOOST_TEST_create_from_plugin<C2DTmeanFilterPlugin>("tmean:w=1,t=1.5");
       P2DImage src_wrap(src_img);
       P2DImage res_wrap = tmean->filter(*src_wrap);
       C2DFImage *res_img = dynamic_cast<C2DFImage *>(res_wrap.get());
       BOOST_REQUIRE(res_img);
       BOOST_REQUIRE(res_img->get_size() == src_img->get_size());

       for (size_t y = 0; y < size_y; ++y)
              for (size_t x = 0; x < size_x; ++x) {
                     if ((*res_img)(x, y) != src_ref[y][x])
                            cvdebug() << "(" << x << "," << y << ")="
                                      << (*res_img)(x, y) << "expect" <<  src_ref[y][x] << "\n";

                     BOOST_CHECK_CLOSE((*res_img)(x, y), src_ref[y][x], 0.01);
              }
}


// For binary images the filter doesn't make sense.
BOOST_AUTO_TEST_CASE( test_2dfilter_tmean_bool )
{
       auto tmean = BOOST_TEST_create_from_plugin<C2DTmeanFilterPlugin>("tmean:w=1,t=1.5");
       P2DImage src_wrap(new C2DBitImage(C2DBounds(7, 5)));
       BOOST_CHECK_THROW(tmean->filter(*src_wrap), invalid_argument);
}


BOOST_AUTO_TEST_CASE( test_2dfilter_tmean_int )
{
       const size_t size_x = 7;
       const size_t size_y = 5;
       const unsigned int src[size_y][size_x] = {
              { 0, 1, 2, 3, 2, 3, 5},
              { 2, 5, 2, 3, 5, 3, 2},
              { 1, 2, 7, 6, 4, 2, 1},
              { 3, 4, 4, 3, 4, 3, 2},
              { 1, 3, 2, 4, 5, 6, 2}
       };
       // "hand filtered" w = 1 -> 3x3 using zero boundary conditions
       //
       const unsigned int src_ref[size_y][size_x] = {
              { 0,  0, 3, 3, 3, 3, 3},
              { 3,  3, 4, 4, 3, 3, 3},
              { 0,  4, 4, 4, 4, 3, 0},
              { 3,  4, 4, 4, 4, 4, 3},
              { 0,  3, 3, 4, 4, 4, 3}
       };
       C2DBounds size(size_x, size_y);
       C2DUIImage *src_img = new C2DUIImage(size);

       for (size_t y = 0; y < size_y; ++y)
              for (size_t x = 0; x < size_x; ++x)
                     (*src_img)(x, y) = src[y][x];

       auto tmean = BOOST_TEST_create_from_plugin<C2DTmeanFilterPlugin>("tmean:w=1,t=1.5");
       P2DImage src_wrap(src_img);
       P2DImage res_wrap = tmean->filter(*src_wrap);
       C2DUIImage *res_img = dynamic_cast<C2DUIImage *>(res_wrap.get());
       BOOST_REQUIRE(res_img);
       BOOST_REQUIRE(res_img->get_size() == src_img->get_size());

       for (size_t y = 0; y < size_y; ++y)
              for (size_t x = 0; x < size_x; ++x) {
                     BOOST_CHECK_EQUAL((*res_img)(x, y), src_ref[y][x]);

                     if ((*res_img)(x, y) != src_ref[y][x])
                            cvdebug() << x << ", " << y << ":= " << (*res_img)(x, y) << " vs " << src_ref[y][x] << "\n";
              }
}
