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
#include <mia/2d/filter/kmeans.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace kmeans_2dimage_filter;


BOOST_AUTO_TEST_CASE( test_2dfilter_kmeans )
{
       const size_t size_x = 5;
       const size_t size_y = 4;
       const float src[size_y * size_x] = {
              1.0, 3.0,  2.0, 10.4,  2.4, 11.0, 11.1, 25.1, 25.2, 28.0,
              29.7, 11.1, 12.2, 28.0, 29.9,  1.0,  2.1, 27.1,  2.5,  3.1
       };
       // "hand filtered" w = 1 -> 3x3
       const int src_ref[size_y * size_x] = {
              0, 0, 0, 1, 0, 1, 1, 2, 2, 2,
              2, 1, 1, 2, 2, 0, 0, 2, 0, 0
       };
       C2DBounds size(size_x, size_y);
       C2DFImage src_img(size, src);
       auto kmeans = BOOST_TEST_create_from_plugin<C2DKMeansFilterPluginFactory>("kmeans:c=3");
       P2DImage res_wrap = kmeans->filter(src_img);
       C2DUBImage *res_img = dynamic_cast<C2DUBImage *>(res_wrap.get());
       BOOST_REQUIRE(res_img);
       BOOST_REQUIRE(res_img->get_size() == src_img.get_size());

       for (size_t y = 0; y < size_y; ++y)
              for (size_t x = 0; x < size_x; ++x)
                     BOOST_CHECK_EQUAL((*res_img)(x, y), src_ref[y * size_x + x]);

       // test attribute
       PAttribute pattr = res_wrap->get_attribute(ATTR_IMAGE_KMEANS_CLASSES);
       const CVDoubleAttribute& attr = dynamic_cast<const CVDoubleAttribute&>(*pattr);
       std::vector<double> cls = attr;
       BOOST_CHECK_EQUAL(cls.size(), cls.size());
}
