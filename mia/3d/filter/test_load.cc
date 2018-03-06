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
#include <mia/3d/filter/load.hh>
#include <mia/core/datapool.hh>
#include <mia/3d/imageio.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace load_3dimage_filter;

struct LoadFixture {
       LoadFixture();
       ~LoadFixture();


       C3DUBImage *orig;
       P3DImage image;
};

LoadFixture::LoadFixture()
{
       const unsigned char init[4] = {1, 2, 3, 4};
       orig = new C3DUBImage(C3DBounds(2, 2, 1), init);
       image.reset(orig);
       save_image("test.@", image);
}

LoadFixture::~LoadFixture()
{
       CDatapool::instance().remove("test.@");
}


BOOST_FIXTURE_TEST_CASE( test_3dfilter_load, LoadFixture )
{
       C3DUBImage dummy(C3DBounds(3, 2, 1));
       auto t = BOOST_TEST_create_from_plugin<C3DLoadFilterPluginFactory>("load:file=test.@");
       auto loaded = t->filter(image);
       BOOST_CHECK(*image == *loaded);
}
