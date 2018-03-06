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

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/3d/transform/translate.hh>

NS_MIA_USE
using namespace std;

struct ipfFixture {
       ipfFixture():
              ipf("bspline:d=3", "mirror")
       {
       }
       C3DInterpolatorFactory ipf;
};


struct TranslateTransformFixture: public ipfFixture {

       TranslateTransformFixture();

       C3DBounds size;
       C3DFVector value;
       C3DTranslateTransformation transf;
};

TranslateTransformFixture::TranslateTransformFixture():
       size(5, 6, 3),
       value(-2, 3, 1),
       transf(size, value, ipf)
{
}

BOOST_FIXTURE_TEST_CASE(test_transform, TranslateTransformFixture)
{
       auto r = transf.transform(C3DFVector(1, 2, 3));
       BOOST_CHECK_CLOSE(r.x, -1, 0.1);
       BOOST_CHECK_CLOSE(r.y,  5, 0.1);
       BOOST_CHECK_CLOSE(r.z,  4, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_upscale, TranslateTransformFixture)
{
       C3DBounds new_size(10, 12, 9);
       auto r = transf.upscale(new_size);
       auto a = dynamic_cast<const C3DTranslateTransformation&>(*r);
       BOOST_CHECK_EQUAL(a.get_size(), new_size);
       auto params = a.get_parameters();
       BOOST_CHECK_EQUAL(params.size(), 3u);
       params[0] = -4;
       params[1] =  6;
       params[1] =  3;
}

BOOST_FIXTURE_TEST_CASE(test_gradtranslate, TranslateTransformFixture)
{
       C3DFVectorfield field(size);
       C3DFVector f(0, 0, 0);

       for (size_t z = 0; z < size.z; ++z)
              for (size_t y = 0; y < size.y; ++y)
                     for (size_t x = 0; x < size.x; ++x) {
                            field(x, y, z) = C3DFVector(x, y, z);
                            f.x -= x;
                            f.y -= y;
                            f.z -= z;
                     }

       CDoubleVector a(3);
       transf.translate(field, a);
       BOOST_CHECK_CLOSE(a[0], f.x, 0.1);
       BOOST_CHECK_CLOSE(a[1], f.y, 0.1);
       BOOST_CHECK_CLOSE(a[2], f.z, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_get_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       BOOST_CHECK_EQUAL(a.size(), 3u);
       BOOST_CHECK_EQUAL(a[0], -2);
       BOOST_CHECK_EQUAL(a[1],  3);
       BOOST_CHECK_EQUAL(a[2],  1);
}

BOOST_FIXTURE_TEST_CASE(test_set_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       a[0] = 2;
       a[1] = -1;
       a[2] = 4;
       transf.set_parameters(a);
       auto b = transf.get_parameters();
       BOOST_CHECK_EQUAL(b.size(), 3u);
       BOOST_CHECK_EQUAL(b[0], a[0]);
       BOOST_CHECK_EQUAL(b[1], a[1]);
       BOOST_CHECK_EQUAL(b[2], a[2]);
}

BOOST_FIXTURE_TEST_CASE(test_invert_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       a[0] = 2;
       a[1] = -1;
       a[2] = 3;
       transf.set_parameters(a);
       unique_ptr<C3DTransformation> inverse( transf.invert());
       BOOST_CHECK_EQUAL(inverse->get_size(), size);
       auto b = inverse->get_parameters();
       BOOST_CHECK_EQUAL(b.size(), 3u);
       BOOST_CHECK_EQUAL(b[0], -a[0]);
       BOOST_CHECK_EQUAL(b[1], -a[1]);
       BOOST_CHECK_EQUAL(b[2], -a[2]);
}

BOOST_FIXTURE_TEST_CASE( test_translate3d_ranged_iterator, ipfFixture)
{
       C3DBounds size(10, 20, 30);
       C3DBounds delta(1, 2, 3);
       C3DFVector value(5, 4, 3);
       C3DTranslateTransformation transf(size, value, ipf);
       auto ti = transf.begin_range(delta, size - delta);

       for (size_t z = delta.z; z < size.z - delta.z; ++z)
              for (size_t y = delta.y; y < size.y - delta.y; ++y)
                     for (size_t x = delta.x; x < size.x - delta.x; ++x, ++ti) {
                            BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z) - value);
                     }

       BOOST_CHECK(ti == transf.end_range(delta, size - delta));
}


BOOST_AUTO_TEST_CASE(test_shift_image)
{
       float src_image_init[10 * 9 * 2] = {

              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 10, 30, 30, 0, 0, 0,
              0, 0, 0, 0, 50, 50, 50, 0, 0, 0,
              0, 0, 0, 0, 50, 50, 50, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       float ref_image_init[10 * 9 * 2] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 10, 30, 30, 0, 0,
              0, 0, 0, 0, 0, 50, 50, 50, 0, 0,
              0, 0, 0, 0, 0, 50, 50, 50, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0

       };
       const C3DBounds size(10, 9, 2);
       P3DImage src(new C3DFImage(size, src_image_init));
       C3DInterpolatorFactory ipf("bspline:d=0", "mirror");
       C3DTranslateTransformation trans(size,  C3DFVector(1.0, 2.0, -1), ipf);
       P3DImage result = trans(*src);
       const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result);
       BOOST_REQUIRE(r.get_size() == size);
       float *itest = ref_image_init;
       auto ir = r.begin();

       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x, ++ir, ++itest)
                     BOOST_CHECK_EQUAL(*ir, *itest);
};
