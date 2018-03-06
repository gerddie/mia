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
#include <mia/3d/filter/thinning.hh>

using namespace thinning_3dimage_filter;
using namespace mia;


BOOST_AUTO_TEST_CASE( test_simple )
{
       C3DBounds size(7, 8, 4);
       bool init[7 * 8 * 4] =  {0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 1, 1, 1, 1, 1, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0,

                                0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 1, 1, 1, 0, 0,
                                0, 1, 1, 1, 1, 1, 0,
                                0, 0, 1, 1, 1, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0,

                                0, 0, 0, 0, 0, 0, 0,
                                0, 1, 1, 1, 0, 0, 0,
                                0, 1, 0, 1, 1, 0, 0,
                                0, 1, 1, 1, 1, 1, 0,
                                0, 1, 1, 1, 1, 1, 0,
                                0, 0, 1, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0,

                                0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 0, 1, 0, 0, 0,
                                0, 0, 1, 0, 1, 0, 0,
                                0, 0, 1, 0, 1, 1, 0,
                                0, 0, 1, 1, 0, 1, 0,
                                0, 0, 0, 0, 1, 1, 0,
                                0, 0, 0, 0, 0, 0, 0
                               };
       const bool test[7 * 8 * 4] =  {0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,

                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,

                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 1, 0, 0, 0, 0,
                                      0, 1, 0, 1, 0, 0, 0,
                                      0, 1, 0, 1, 0, 0, 0,
                                      0, 0, 1, 1, 0, 0, 0,
                                      0, 0, 0, 1, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,

                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 1, 0, 0,
                                      0, 0, 0, 0, 0, 1, 0,
                                      0, 0, 0, 0, 1, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0
                                     };
       auto thinning = BOOST_TEST_create_from_plugin<C3DThinningFilterPlugin>("thinning");
       C3DBitImage input(size, init);
       auto thin = thinning->filter(input);
       auto res_img = dynamic_cast<const C3DBitImage&>(*thin);
       BOOST_REQUIRE(res_img.get_size() == size);
       int i = 0;

       for (auto r = res_img.begin_range(C3DBounds::_0, size); r != res_img.end_range(C3DBounds::_0, size); ++r, ++i) {
              BOOST_CHECK_EQUAL(*r, test[i]);

              if (*r != test[i])
                     cvfail() << r.pos() << "\n";
       }
}
