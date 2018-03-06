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
#include <mia/2d/filter/thresh.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace thresh_2dimage_filter;

struct ThreshFixture {

       ThreshFixture();
       void run_test(const char *param, const int *test_data);

       static const int src[];
       static const int ref_4[];
       static const int thresh;

       static const C2DBounds size;
       P2DImage src_img;

};


const C2DBounds ThreshFixture::size = C2DBounds(7, 5);
const int    ThreshFixture::thresh = 5;


const int ThreshFixture::src[] = {
       0, 1, 2, 3, 2, 3, 5,
       2, 5, 2, 3, 5, 3, 2,
       1, 2, 7, 6, 4, 2, 1,
       3, 4, 4, 3, 4, 3, 2,
       1, 3, 2, 4, 5, 6, 2
};


const int ThreshFixture::ref_4[] = {
       0, 1, 0, 0, 2, 3, 5,
       2, 5, 2, 3, 5, 3, 2,
       0, 2, 7, 6, 4, 0, 0,
       0, 0, 4, 3, 4, 3, 0,
       0, 0, 0, 4, 5, 6, 2
};


ThreshFixture::ThreshFixture():
       src_img(new C2DSIImage(size, src))
{
}

void ThreshFixture::run_test(const char *param, const int *test_data)
{
       auto thresh = BOOST_TEST_create_from_plugin<C2DThreshNImageFilterFactory>(param);
       P2DImage result = thresh->filter(*src_img);
       BOOST_REQUIRE(result);
       BOOST_REQUIRE(result->get_size() == src_img->get_size());
       const C2DSIImage& rimg = dynamic_cast<const C2DSIImage&>(*result);

       for (auto ir = rimg.begin(); ir != rimg.end(); ++ir, ++test_data)
              BOOST_CHECK_EQUAL(*ir, *test_data);
}

BOOST_FIXTURE_TEST_CASE( test_thresh_n4, ThreshFixture)
{
       run_test("thresh:thresh=5,shape=4n", ref_4);
}


