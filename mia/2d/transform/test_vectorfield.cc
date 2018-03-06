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
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/transform/vectorfield.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;

struct GridTransformFixture {
       GridTransformFixture():
              size(256, 128),
              r(size.x - 1, size.y - 1),
              field(size, C2DInterpolatorFactory("bspline:d=1", "mirror")),
              scale(2 * M_PI / r.x, 2 * M_PI / r.y)
       {
              C2DGridTransformation::field_iterator i = field.field_begin();

              for (size_t y = 0; y < size.y; ++y)
                     for (size_t x = 0; x < size.x; ++x, ++i) {
                            *i = C2DFVector( fx(x, y), fy(x, y));
                     }
       }
       C2DBounds size;
       C2DBounds r;
       C2DGridTransformation field;
protected:
       float fx(float x, float y);
       float fy(float x, float y);

       float dfx_x(float x, float y);
       float dfy_x(float x, float y);
       float dfx_y(float x, float y);
       float dfy_y(float x, float y);

       float dfx_xx(float x, float y);
       float dfx_xy(float x, float y);
       float dfx_yx(float x, float y);
       float dfx_yy(float x, float y);
       float dfy_xx(float x, float y);
       float dfy_xy(float x, float y);
       float dfy_yx(float x, float y);
       float dfy_yy(float x, float y);

       C2DFVector scale;
};


BOOST_FIXTURE_TEST_CASE(test_gridtransform_basic_props, GridTransformFixture)
{
       BOOST_CHECK_EQUAL(field.degrees_of_freedom(), size.x * size.y * 2);
       C2DFVector testx(20.2, 21.9);
       C2DFVector result = field.get_displacement_at(testx);
       BOOST_CHECK_CLOSE(result.x, fx(testx.x, testx.y), 1);
       BOOST_CHECK_CLOSE(result.y, fy(testx.x, testx.y), 1);
       result = field(testx);
       BOOST_CHECK_CLOSE(result.x, testx.x - fx(testx.x, testx.y), 0.1);
       BOOST_CHECK_CLOSE(result.y, testx.y - fy(testx.x, testx.y), 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative, GridTransformFixture)
{
       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x) {
                     C2DFMatrix dv =  field.derivative_at(x, y);

                     if ( x > 0 && x < size.x - 1) {
                            BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x, y), 1);
                            float test_value = -dfy_x(x, y);

                            if (fabs(test_value) < 1e-10)
                                   BOOST_CHECK_SMALL(dv.x.y, 1e-10f);
                            else
                                   BOOST_CHECK_CLOSE(dv.x.y, test_value, 0.1);
                     } else {
                            BOOST_CHECK_CLOSE(dv.x.x, 1.0f, 0.1);
                            BOOST_CHECK_SMALL(dv.x.y, 1e-10f);
                     }

                     if ( y > 0 && y < size.y - 1) {
                            float test_value = -dfx_y(x, y);

                            if (fabs(test_value) < 1e-10)
                                   BOOST_CHECK_SMALL(dv.y.x, 1e-10f);
                            else
                                   BOOST_CHECK_CLOSE(dv.y.x, test_value, 1);

                            BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x, y), 1);
                     } else {
                            BOOST_CHECK_SMALL(dv.y.x, 1e-10f);
                            BOOST_CHECK_CLOSE(dv.y.y, 1.0f, 0.1);
                     }
              }
}


BOOST_FIXTURE_TEST_CASE(test_gridtransform_derivative_at, GridTransformFixture)
{
       const float x = 200.3;
       const float y = 68.2;
       C2DFMatrix dv =  field.derivative_at(C2DFVector(x, y));
       BOOST_CHECK_CLOSE(dv.x.x, 1.0f - dfx_x(x, y), 1);
       BOOST_CHECK_CLOSE(dv.x.y, -dfy_x(x, y), 1);
       BOOST_CHECK_CLOSE(dv.y.x, -dfx_y(x, y), 1);
       BOOST_CHECK_CLOSE(dv.y.y, 1.0f - dfy_y(x, y), 1);
}

BOOST_FIXTURE_TEST_CASE(test_gridtransform_set_identity, GridTransformFixture)
{
       field.set_identity();
       C2DGridTransformation::const_iterator i = field.begin();

       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x, ++i) {
                     BOOST_CHECK_EQUAL( *i, C2DFVector(x, y));
              }
}


BOOST_AUTO_TEST_CASE( test_grid2d_iterator )
{
       C2DBounds size(5, 10);
       C2DInterpolatorFactory ipf("bspline:d=1", "mirror");
       C2DGridTransformation gt(size, ipf);
       C2DGridTransformation::field_iterator i = gt.field_begin();
       C2DFVector value(0.12, 0.32);

       while (i != gt.field_end()) {
              *i = value;
              value.x += 0.12f;
              value.y += 0.32f;
              ++i;
       }

       C2DTransformation::const_iterator k = gt.begin();
       float n = 1.0f;

       for (size_t y = 0; y < size.y; ++y)
              for (size_t x = 0; x < size.x; ++x, ++k, n += 1.0f) {
                     BOOST_CHECK_CLOSE( k->x, (float)x - n * 0.12f, 0.1f );
                     BOOST_CHECK_CLOSE( k->y, (float)y - n * 0.32f, 0.1f );
              }
}

BOOST_AUTO_TEST_CASE( test_gridtransform_add )
{
       C2DFVector init_a[9] = {
              C2DFVector(2, 3), C2DFVector(0, 0), C2DFVector(0, 0),
              C2DFVector(0, 0), C2DFVector(-1, -1), C2DFVector(0, 0),
              C2DFVector(0, 0), C2DFVector(0, 0), C2DFVector(0, 0)
       };
       C2DFVector init_b[9] = {
              C2DFVector(0, 0), C2DFVector(0, 0), C2DFVector(0, 0),
              C2DFVector(0, 0), C2DFVector(1, 1), C2DFVector(0.5, 0),
              C2DFVector(0, 0), C2DFVector(0, 0.5), C2DFVector(4, 5)
       };
       C2DBounds size(3, 3);
       C2DInterpolatorFactory ipf("bspline:d=1", "mirror");
       C2DGridTransformation a(size, ipf);
       std::copy(init_a, init_a + 9, a.field_begin());
       C2DGridTransformation b(size, ipf);
       std::copy(init_b, init_b + 9, b.field_begin());
       C2DGridTransformation c = a + b;
       BOOST_CHECK_EQUAL( c.get_displacement_at(C2DFVector(1, 1)), C2DFVector(3, 4));
       BOOST_CHECK_EQUAL( c.get_displacement_at(C2DFVector(2, 1)), C2DFVector(0.0, -0.5));
       BOOST_CHECK_EQUAL( c.get_displacement_at(C2DFVector(1, 2)), C2DFVector(-0.5, 0.0));
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_max, GridTransformFixture )
{
       float fx0 = fx(0.75 * size.x, size.y / 2);
       float fy0 = fy(0.75 * size.x, size.y / 2);
       BOOST_CHECK_CLOSE(sqrt(fx0 * fx0 + fy0 * fy0), field.get_max_transform(), 1);
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_pertuberate, GridTransformFixture )
{
       C2DFVectorfield v(size);
       C2DFVector vv(10.0, 20.0);
       fill(v.begin(), v.end(), vv);
       float gamma = field.pertuberate(v);
       C2DFVector lmg(184, 107);
       C2DFVector mg(vv.x - vv.x * dfx_x(lmg.x, lmg.y) - vv.y * dfx_y(lmg.x, lmg.y),
                     vv.y - vv.x * dfy_x(lmg.x, lmg.y) - vv.y * dfy_y(lmg.x, lmg.y));
       BOOST_CHECK_CLOSE(gamma, mg.norm(), 0.1);

       for (size_t y = 1; y < size.y - 1; ++y)
              for (size_t x = 1; x < size.x - 1; ++x) {
                     const C2DFVector& iv = v(x, y);
                     BOOST_CHECK_CLOSE(iv.x, vv.x - vv.x * dfx_x(x, y) - vv.y * dfx_y(x, y), 1);
                     BOOST_CHECK_CLOSE(iv.y, vv.y - vv.x * dfy_x(x, y) - vv.y * dfy_y(x, y), 1);
              }
}

BOOST_FIXTURE_TEST_CASE( test_gridtransform_get_jacobian, GridTransformFixture )
{
       C2DFVectorfield v(size);
       fill(v.begin(), v.end(), C2DFVector(10.0, 20.0));
       C2DFVector lmg(64, 32);
       float j = field.get_jacobian(v, 1.0);
       C2DFMatrix J = field.derivative_at(lmg.x, lmg.y);
       BOOST_CHECK_CLOSE(j, J.x.x * J.y.y - J.x.y * J.y.x, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_grid_clone, GridTransformFixture )
{
       P2DTransformation clone(field.clone());
       C2DGridTransformation& spclone = dynamic_cast<C2DGridTransformation&>(*clone);
       BOOST_CHECK_EQUAL(spclone.get_size(), field.get_size());
       C2DGridTransformation::const_iterator ic = spclone.begin();
       C2DGridTransformation::const_iterator ec = spclone.end();
       C2DGridTransformation::const_iterator io = field.begin();

       while (ic != ec ) {
              BOOST_CHECK_EQUAL(*ic, *io);
              ++ic;
              ++io;
       }
}

struct DivGradFixture {

       DivGradFixture();
       const int dsize;
       const float  range;
       C2DBounds size;
       float scale;
       float scale2;
       float corr;
       C2DGridTransformation field;
       double gx(double x, double y) const;
       double gy(double x, double y) const;

       double ddgx_xx(double x, double y) const;
       double ddgx_yy(double x, double y) const;
       double ddgy_xx(double x, double y) const;
       double ddgy_yy(double x, double y) const;

       double ddgx_xy(double x, double y) const;
       double ddgy_xy(double x, double y) const;

       double dddgx_xxx(double x, double y) const;
       double dddgy_yyy(double x, double y) const;

       double dddgx_xxy(double x, double y) const;
       double dddgy_xxy(double x, double y) const;
       double dddgx_yyx(double x, double y) const;
       double dddgy_yyx(double x, double y) const;


};

DivGradFixture::DivGradFixture():
       dsize(128),
       range(4.0),
       size(2 * dsize + 1, 2 * dsize + 1),
       scale(range / dsize),
       scale2(scale * scale),
       corr(dsize / range),
       field(size, C2DInterpolatorFactory("bspline:d=1", "mirror") )
{
       auto i = field.field_begin();

       for (int y = 0; y < (int)size.y; ++y)
              for (int x = 0; x < (int)size.x; ++x, ++i) {
                     i->x = gx(x, y);
                     i->y = gy(x, y);
              }
}

double DivGradFixture::gx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return fx * exp(-fx * fx - fy * fy);
}
double DivGradFixture::gy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return fy * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgx_xx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fx * (2 * fx * fx - 3 ) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgx_yy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fx * (2 * fy * fy - 1 ) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgy_xx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fy * (2 * fx * fx - 1 ) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgy_yy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fy * (2 * fy * fy - 3 ) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgx_xy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fy * (2 * fx * fx - 1) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::ddgy_xy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fx * (2 * fy * fy - 1) * exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgx_xxx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 2 * scale2 * scale * (4 * fx * fx * fx * fx -
                                      12 * fx * fx + 3)
              * exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgy_yyy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 2 * scale2 * scale * (4 * fy * fy * fy * fy -
                                      12 * fy * fy + 3)
              * exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgx_xxy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -4 * scale * scale2 * fx * fy * (2 * fx * fx - 3)
              * exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgy_xxy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 2 * scale * scale2 * (2 * fx * fx - 1) * (2 * fy * fy - 1) *
              exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgx_yyx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 2 * scale * scale2 * (2 * fx * fx - 1) * (2 * fy * fy - 1) *
              exp(-fx * fx - fy * fy);
}

double DivGradFixture::dddgy_yyx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -4 * scale * scale2 * fx * fy * (2 * fy * fy - 3)
              * exp(-fx * fx - fy * fy);
}

struct CurlGradFixture {

       CurlGradFixture();
       const int dsize;
       const float  range;
       C2DBounds size;
       float scale;
       float scale2;
       float corr;
       C2DGridTransformation field;
       double gx(double x, double y) const;
       double gy(double x, double y) const;

       double ddgx_xx(double x, double y) const;
       double ddgx_yy(double x, double y) const;
       double ddgy_xx(double x, double y) const;
       double ddgy_yy(double x, double y) const;

       double ddgx_xy(double x, double y) const;
       double ddgy_xy(double x, double y) const;

       double dddgx_xxx(double x, double y) const;
       double dddgy_yyy(double x, double y) const;

       double dddgx_xxy(double x, double y) const;
       double dddgy_xxy(double x, double y) const;
       double dddgx_yyx(double x, double y) const;
       double dddgy_yyx(double x, double y) const;


};

CurlGradFixture::CurlGradFixture():
       dsize(128),
       range(4.0),
       size(2 * dsize + 1, 2 * dsize + 1),
       scale(range / dsize),
       scale2(scale * scale),
       corr(dsize / range),
       field(size, C2DInterpolatorFactory("bspline:d=3", "mirror"))
{
       auto i = field.field_begin();

       for (int y = 0; y < (int)size.y; ++y)
              for (int x = 0; x < (int)size.x; ++x, ++i) {
                     i->x = gx(x, y);
                     i->y = gy(x, y);
              }
}

double CurlGradFixture::gx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return fy * exp(-fx * fx - fy * fy);
}
double CurlGradFixture::gy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -fx * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgx_xx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fy * (2 * fx * fx - 1) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgx_yy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fy * (2 * fy * fy - 3) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgy_xx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -2 * scale2 * fx * (2 * fx * fx - 3) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgy_yy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -2 * scale2 * fx * (2 * fy * fy - 1) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgx_xy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale2 * fx * (2 * fy * fy - 1) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::ddgy_xy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 2 * scale2 * fy * (2 * fx * fx - 1) * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgx_xxx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 4 * fx * fy * scale2 * scale * (2 * fx * fx - 3)
              * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgy_yyy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 4 * fx * fy * scale2 * scale * (2 * fy * fy - 3)
              * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgx_xxy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return -2 * scale * scale2
              * (2 * fx * fx - 1)
              * (2 * fy * fy - 1)
              * exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgy_xxy(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 4 * scale * scale2 * (2 * fx * fx - 3) * fx * fy *
              exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgx_yyx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return - 4 * scale * scale2 * fx * fy * (2 * fy * fy - 3) *
              exp(-fx * fx - fy * fy);
}

double CurlGradFixture::dddgy_yyx(double x, double y) const
{
       const double fx = scale * (x - dsize);
       const double fy = scale * (y - dsize);
       return 2 * scale * scale2
              * (2 * fx * fx - 1)
              * (2 * fy * fy - 1)
              * exp(-fx * fx - fy * fy);
}


BOOST_FIXTURE_TEST_CASE( CurlGradFixture_selftest, CurlGradFixture )
{
       const double x = 120.0;
       const double y = 131.0;
       BOOST_CHECK_CLOSE(dddgx_xxx(x, y),
                         (ddgx_xx(x + 0.001, y) -  ddgx_xx(x - 0.001, y)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgx_xxy(x, y),
                         (ddgx_xx(x, y + 0.001) -  ddgx_xx(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgx_yyx(x, y),
                         (ddgx_xy(x, y + 0.001) -  ddgx_xy(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_yyy(x, y),
                         (ddgy_yy(x, y + 0.001) -  ddgy_yy(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_yyx(x, y),
                         (ddgy_yy(x + 0.001, y) -  ddgy_yy(x - 0.001, y)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_xxy(x, y),
                         (ddgy_xy(x + 0.001, y) -  ddgy_xy(x - 0.001, y)) / 0.002,
                         0.1);
}


BOOST_FIXTURE_TEST_CASE( DivGradFixture_selftest, DivGradFixture )
{
       const double x = 120.0;
       const double y = 131.0;
       BOOST_CHECK_CLOSE(dddgx_xxx(x, y),
                         (ddgx_xx(x + 0.001, y) -  ddgx_xx(x - 0.001, y)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgx_xxy(x, y),
                         (ddgx_xx(x, y + 0.001) -  ddgx_xx(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgx_yyx(x, y),
                         (ddgx_xy(x, y + 0.001) -  ddgx_xy(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_yyy(x, y),
                         (ddgy_yy(x, y + 0.001) -  ddgy_yy(x, y - 0.001)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_yyx(x, y),
                         (ddgy_yy(x + 0.001, y) -  ddgy_yy(x - 0.001, y)) / 0.002,
                         0.1);
       BOOST_CHECK_CLOSE(dddgy_xxy(x, y),
                         (ddgy_xy(x + 0.001, y) -  ddgy_xy(x - 0.001, y)) / 0.002,
                         0.1);
}


float GridTransformFixture::fx(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return 	(1.0 + sinf(x - M_PI / 2.0)) * (1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::fy(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return (1.0 - cosf(2 * x)) * (1.0 - cosf(y));
}


float GridTransformFixture::dfx_x(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return scale.x * cosf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::dfx_y(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return 4.0 * M_PI / r.y * (1.0 + sinf(x - M_PI / 2.0)) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfy_x(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return 4 * M_PI / r.x * sinf(2 * x) * (1.0 - cosf(y));
}

float GridTransformFixture::dfy_y(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return scale.y * (1.0 - cosf(2 * x)) * sinf(y);
}


float GridTransformFixture::dfx_xx(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return - scale.x * scale.x * sinf(x - M_PI / 2.0) * ( 1.0 + sinf(2 * y  - M_PI / 2.0));
}

float GridTransformFixture::dfx_xy(float x, float y)

{
       x *= scale.x;
       y *= scale.y;
       return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yx(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return 2.0 * scale.x * scale.y * cosf(x - M_PI / 2.0) * cosf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfx_yy(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return -4.0 * scale.y * scale.y * (1.0 + sinf(x - M_PI / 2.0)) * sinf(2 * y  - M_PI / 2.0);
}

float GridTransformFixture::dfy_xx(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return -4.0 * scale.x * scale.x * cosf(2 * x) * (1.0 - cosf(y));
}

float GridTransformFixture::dfy_xy(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return -2.0 * scale.x * scale.y * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yx(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return -2.0 * scale.y * scale.x * sinf(2 * x) * sinf(y);
}

float GridTransformFixture::dfy_yy(float x, float y)
{
       x *= scale.x;
       y *= scale.y;
       return - scale.y * scale.y * (1.0 - cosf(2 * x)) * cosf(y);
}
