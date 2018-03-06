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
#include <mia/2d/filter/tee.hh>
#include <mia/core/datapool.hh>
#include <mia/2d/imageio.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace tee_2dimage_filter;

struct TeeFixture {
       TeeFixture();
       ~TeeFixture();


       C2DUBImage *orig;
       P2DImage image;
};

TeeFixture::TeeFixture()
{
       const unsigned char init[4] = {1, 2, 3, 4};
       orig = new C2DUBImage(C2DBounds(2, 2), init);
       image.reset(orig);
}

TeeFixture::~TeeFixture()
{
       CDatapool::instance().remove("test.@");
}


BOOST_FIXTURE_TEST_CASE( test_2dfilter_tee_shared_ptr, TeeFixture )
{
       auto t = BOOST_TEST_create_from_plugin<C2DTeeFilterPluginFactory>("tee:file=test.@");
       auto passthrough = t->filter(image);
       BOOST_CHECK(*image == *passthrough);
       auto loaded = load_image2d("test.@");
       BOOST_CHECK(*image == *loaded);
}

BOOST_FIXTURE_TEST_CASE( test_2dfilter_tee, TeeFixture )
{
       auto t = BOOST_TEST_create_from_plugin<C2DTeeFilterPluginFactory>("tee:file=test.@");
       auto passthrough2 = t->filter(*image);
       BOOST_CHECK(*image == *passthrough2);
       auto loaded2 = load_image2d("test.@");
       BOOST_CHECK(*image == *loaded2);
}
