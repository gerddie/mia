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

#include <mia/2d/transform/translate.hh>

NS_MIA_USE
using namespace std;

struct TranslateTransformFixture {

       TranslateTransformFixture();
       C2DInterpolatorFactory ipf;
       C2DBounds size;
       C2DFVector value;
       C2DTranslateTransformation transf;
};

TranslateTransformFixture::TranslateTransformFixture():
       ipf("bspline:d=3", "mirror"),
       size(5, 6),
       value(-2, 3),
       transf(size, value, ipf)
{
}

BOOST_FIXTURE_TEST_CASE(test_transform, TranslateTransformFixture)
{
       auto r = transf.transform(C2DFVector(1, 2));
       BOOST_CHECK_CLOSE(r.x, -1, 0.1);
       BOOST_CHECK_CLOSE(r.y,  5, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_transform_derivative_at_gridpoint, TranslateTransformFixture)
{
       auto jac = transf.derivative_at(3, 4);
       BOOST_CHECK_EQUAL(jac.x.x, 1.0f);
       BOOST_CHECK_EQUAL(jac.y.y, 1.0f);
       BOOST_CHECK_EQUAL(jac.x.y, 0.0f);
       BOOST_CHECK_EQUAL(jac.y.x, 0.0f);
}

BOOST_FIXTURE_TEST_CASE(test_transform_derivative, TranslateTransformFixture)
{
       auto jac = transf.derivative_at(C2DFVector(3.1, 4.2));
       BOOST_CHECK_EQUAL(jac.x.x, 1.0f);
       BOOST_CHECK_EQUAL(jac.y.y, 1.0f);
       BOOST_CHECK_EQUAL(jac.x.y, 0.0f);
       BOOST_CHECK_EQUAL(jac.y.x, 0.0f);
}

BOOST_FIXTURE_TEST_CASE(test_upscale, TranslateTransformFixture)
{
       C2DBounds new_size(10, 12);
       auto r = transf.upscale(new_size);
       auto a = dynamic_cast<const C2DTranslateTransformation&>(*r);
       BOOST_CHECK_EQUAL(a.get_size(), new_size);
       auto params = a.get_parameters();
       BOOST_CHECK_EQUAL(params.size(), 2u);
       params[0] = -4;
       params[1] =  6;
}

BOOST_FIXTURE_TEST_CASE(test_gradtranslate, TranslateTransformFixture)
{
       C2DFVectorfield field(size);
       C2DFVector f(0, 0);

       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x) {
                     field(x, y) = C2DFVector(x, y);
                     f.x -= x;
                     f.y -= y;
              }

       CDoubleVector a(2);
       transf.translate(field, a);
       BOOST_CHECK_CLOSE(a[0], f.x, 0.1);
       BOOST_CHECK_CLOSE(a[1], f.y, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_get_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       BOOST_CHECK_EQUAL(a.size(), 2u);
       BOOST_CHECK_EQUAL(a[0], -2);
       BOOST_CHECK_EQUAL(a[1],  3);
}

BOOST_FIXTURE_TEST_CASE(test_set_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       a[0] = 2;
       a[1] = -1;
       transf.set_parameters(a);
       auto b = transf.get_parameters();
       BOOST_CHECK_EQUAL(b.size(), 2u);
       BOOST_CHECK_EQUAL(b[0], a[0]);
       BOOST_CHECK_EQUAL(b[1], a[1]);
}

BOOST_FIXTURE_TEST_CASE(test_invert_params, TranslateTransformFixture)
{
       auto a = transf.get_parameters();
       a[0] = 2;
       a[1] = -1;
       transf.set_parameters(a);
       unique_ptr<C2DTransformation> inverse( transf.invert());
       BOOST_CHECK_EQUAL(inverse->get_size(), size);
       auto b = inverse->get_parameters();
       BOOST_CHECK_EQUAL(b.size(), 2u);
       BOOST_CHECK_EQUAL(b[0], -a[0]);
       BOOST_CHECK_EQUAL(b[1], -a[1]);
}

BOOST_AUTO_TEST_CASE(test_shift_image)
{
       float src_image_init[10 * 9] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 1, 2, 3, 0, 0, 0,
              0, 0, 0, 0, 10, 30, 30, 0, 0, 0,
              0, 0, 0, 0, 50, 50, 50, 0, 0, 0,
              0, 0, 0, 20, 50, 50, 50, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       float ref_image_init[10 * 9] = {
              0, 0, 0, 0, 0, 10, 30, 30, 0, 0,
              0, 0, 0, 0, 0, 1, 2, 3, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 1, 2, 3, 0, 0,
              0, 0, 0, 0, 0, 10, 30, 30, 0, 0,
              0, 0, 0, 0, 0, 50, 50, 50, 0, 0,
              0, 0, 0, 0, 20, 50, 50, 50, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       };
       const C2DBounds size(10, 9);
       P2DImage src(new C2DFImage(size, src_image_init));
       C2DInterpolatorFactory ipf("bspline:d=0", "mirror");
       C2DTranslateTransformation transform(size,  C2DFVector(1.0, 2.0), ipf);
       C2DTransformation& help = transform;
       P2DImage result = help(*src);
       const C2DFImage& r = dynamic_cast<const C2DFImage&>(*result);
       BOOST_REQUIRE(r.get_size() == size);
       float *itest = ref_image_init;
       auto ir = r.begin();

       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x, ++ir, ++itest)
                     BOOST_CHECK_EQUAL(*ir, *itest);
};
