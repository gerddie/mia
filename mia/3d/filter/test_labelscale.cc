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
#include <mia/3d/filter/labelscale.hh>

using namespace labelscale_3dimage_filter;
using namespace mia;

BOOST_AUTO_TEST_CASE ( test_labelscale_downscale )
{
       const C3DBounds in_size(10, 4, 6);
       const unsigned char in_image[240] = {
              1, 1, 1, 2, 2, 2, 2, 3, 3, 3,
              1, 1, 2, 2, 2, 2, 2, 3, 3, 3,
              1, 1, 1, 2, 2, 3, 3, 3, 4, 4,
              5, 5, 5, 2, 2, 2, 3, 3, 4, 4,

              1, 1, 1, 2, 2, 3, 3, 3, 3, 3,
              1, 1, 1, 2, 7, 2, 3, 3, 3, 3,
              5, 6, 5, 7, 7, 7, 7, 3, 4, 4,
              6, 6, 6, 7, 7, 7, 7, 3, 4, 4,

              1, 1, 1, 7, 7, 7, 7, 3, 3, 3,
              1, 1, 1, 8, 7, 7, 7, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,

              1, 1, 1, 7, 6, 6, 6, 3, 3, 3,
              1, 1, 1, 8, 6, 6, 6, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,


              1, 1, 1, 7, 6, 6, 6, 3, 3, 3,
              1, 1, 1, 8, 7, 6, 6, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 6, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,

              1, 1, 1, 7, 6, 6, 6, 3, 3, 3,
              1, 1, 1, 8, 6, 6, 6, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,
              8, 8, 8, 8, 7, 7, 7, 7, 3, 3,


       };
       const C3DBounds out_size(5, 2, 3);
       const unsigned char out_image[30] = {
              1, 2, 2, 3, 3,
              5, 2, 7, 3, 4,
              1, 1, 6, 7, 3,

              8, 8, 7, 7, 3,
              1, 1, 6, 6, 3,
              8, 8, 7, 7, 3

       };
       auto f = BOOST_TEST_create_from_plugin<C3DLabelscaleFilterPluginFactory>("labelscale:out-size=[5,2,3]");
       C3DUBImage fimage(in_size, in_image );
       fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));
       P3DImage scaled = f->filter(fimage);
       BOOST_CHECK_EQUAL(scaled->get_size(), out_size);
       const C3DUBImage& fscaled = dynamic_cast<const C3DUBImage& >(*scaled);
       BOOST_REQUIRE(scaled->get_size() == out_size);
       BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(4.0f, 6.f, 8.0));

       for (size_t i = 0; i < 30; ++i) {
              cvdebug() << i << ":" << fscaled[i] << " - " << out_image[i] << '\n';
              BOOST_CHECK_EQUAL(fscaled[i], out_image[i]);
       }
}



BOOST_AUTO_TEST_CASE ( test_labelscale_upscale )
{
       const C3DBounds out_size(10, 6, 4);
       const unsigned char out_image[240] = {
              1, 1,   2, 2,   2, 2,   3, 3,   3, 3,
              1, 1,   2, 2,   2, 2,   3, 3,   3, 3,
              5, 5,   2, 2,   7, 7,   3, 3,   3, 3,
              5, 5,   2, 2,   7, 7,   3, 3,   3, 3,
              8, 8,   8, 8,   7, 7,   7, 7,   3, 3,
              8, 8,   8, 8,   7, 7,   7, 7,   3, 3,

              1, 1,   2, 2,   2, 2,   3, 3,   3, 3,
              1, 1,   2, 2,   2, 2,   3, 3,   3, 3,
              5, 5,   2, 2,   7, 7,   3, 3,   3, 3,
              5, 5,   2, 2,   7, 7,   3, 3,   3, 3,
              8, 8,   8, 8,   7, 7,   7, 7,   3, 3,
              8, 8,   8, 8,   7, 7,   7, 7,   3, 3,

              3, 3,   4, 4,   2, 2,   9, 9,   9, 9,
              3, 3,   4, 4,   2, 2,   9, 9,   9, 9,
              5, 5,   2, 2,   2, 2,   3, 3,   6, 6,
              5, 5,   2, 2,   2, 2,   3, 3,   6, 6,
              8, 8,   1, 1,   3, 3,   1, 1,   3, 3,
              8, 8,   1, 1,   3, 3,   1, 1,   3, 3,

              3, 3,   4, 4,   2, 2,   9, 9,   9, 9,
              3, 3,   4, 4,   2, 2,   9, 9,   9, 9,
              5, 5,   2, 2,   2, 2,   3, 3,   6, 6,
              5, 5,   2, 2,   2, 2,   3, 3,   6, 6,
              8, 8,   1, 1,   3, 3,   1, 1,   3, 3,
              8, 8,   1, 1,   3, 3,   1, 1,   3, 3,

       };
       const C3DBounds in_size(5, 3, 2);
       const unsigned char in_image[30] = {
              1,    2,    2,    3,    3,
              5,    2,    7,    3,    3,
              8,    8,    7,    7,    3,
              3,    4,    2,    9,    9,
              5,    2,    2,    3,    6,
              8,    1,    3,    1,    3

       };
       auto f = BOOST_TEST_create_from_plugin<C3DLabelscaleFilterPluginFactory>("labelscale:out-size=[10,6,4]");
       C3DUBImage fimage(in_size, in_image );
       fimage.set_voxel_size(C3DFVector(2.0, 3.0, 4.0));
       P3DImage scaled = f->filter(fimage);
       BOOST_CHECK_EQUAL(scaled->get_size(), out_size);
       const C3DUBImage& fscaled = dynamic_cast<const C3DUBImage& >(*scaled);
       BOOST_REQUIRE(scaled->get_size() == out_size);
       BOOST_CHECK_EQUAL(fscaled.get_voxel_size(), C3DFVector(1.0f, 1.5f, 2.0));

       for (size_t i = 0; i < 240; ++i) {
              cvdebug() << i << ":" << int(fscaled[i]) << " - " << int(out_image[i]) << '\n';
              BOOST_CHECK_EQUAL(fscaled[i], out_image[i]);
       }
}

