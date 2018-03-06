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

#include <mia/internal/autotest.hh>

#include <mia/2d/divcurlmatrix.hh>

NS_MIA_USE;


struct TransformSplineFixtureFieldBase {
       TransformSplineFixtureFieldBase()
       {
       }
       void init(int dsize, float range, EInterpolation type)
       {
              ipf.reset(create_2dinterpolation_factory(type));
              size = C2DBounds(dsize + 1, dsize + 1);
              int dsize2 = dsize / 2;
              field = C2DFVectorfield(size);
              float scale = range / dsize2;
              h = dsize2 / range;
              C2DFVectorfield::iterator i = field.begin();

              for (int y = 0; y < (int)size.y; ++y)
                     for (int x = 0; x < (int)size.x; ++x, ++i) {
                            float sx = (x - dsize2) * scale;
                            float sy = (y - dsize2) * scale;
                            *i = C2DFVector( fx(sx, sy), fy(sx, sy));
                     }

              std::shared_ptr<T2DInterpolator<C2DFVector>> source(ipf->create(field));
       }
       C2DBounds size;
       C2DFVectorfield field;
       P2DInterpolatorFactory ipf;
       C2DBounds range;
       double h;
protected:
       virtual float fx(float x, float y) = 0;
       virtual float fy(float x, float y) = 0;
private:
       C2DFVector scale;
};



struct TransformSplineFixtureConstDivergence: public TransformSplineFixtureFieldBase {
       virtual float fx(float x, float y);
       virtual float fy(float x, float y);
};

struct TransformSplineFixtureexpm2Field: public TransformSplineFixtureFieldBase {
       TransformSplineFixtureexpm2Field() {}
       virtual float fx(float x, float y);
       virtual float fy(float x, float y);

       float dfx_xx(float x, float y);
       float dfx_xy(float x, float y);
       float dfx_yy(float x, float y);

       float dfy_xx(float x, float y);
       float dfy_xy(float x, float y);
       float dfy_yy(float x, float y);

       float dfx_xxx(float x, float y);
       float dfx_xxy(float x, float y);
       float dfx_xyy(float x, float y);
       float dfx_yyy(float x, float y);

       float dfy_xxx(float x, float y);
       float dfy_xxy(float x, float y);
       float dfy_xyy(float x, float y);
       float dfy_yyy(float x, float y);

       float integrate_divcurl(float x1, float x2, float y1, float y2, int xinterv, int yinterv);
       double divcurl_value_at(float x, float y);
       C2DFVector divcurl_derivative_at(float x, float y);
};

#if 0
BOOST_FIXTURE_TEST_CASE( test_divergence_at, TransformSplineFixtureexpm2Field )
{
       init(32, 0.1, ip_bspline5);
       const double testvalue = 6.0 * M_PI;
       C2DDivCurlMatrix divcurl(ipf->get_kernel());
       BOOST_CHECK_CLOSE(h * h * divcurl.value_at(field, 16, 16), divcurl_value_at(0, 0), 1);
       double tx = (24.0 - 16) / h;
       double ty = (24.0 - 16) / h;
       cvdebug() << "h=" << h
                 << ", tx=" << tx
                 << ", ty=" << ty
                 << "\n";
       // evaluated using maxima
       double h0 = h * h * divcurl.value_at(field, 24, 24);
       double hxp = h * h * divcurl.value_at(field, 25, 24);
       double hxm = h * h * divcurl.value_at(field, 23, 24);
       double hyp = h * h * divcurl.value_at(field, 24, 25);
       double hym = h * h * divcurl.value_at(field, 24, 23);
       BOOST_CHECK_CLOSE( h0, divcurl_value_at(tx, ty), 1);
       BOOST_CHECK_CLOSE(hxm, divcurl_value_at(tx - 1.0 / h, ty), 1);
       BOOST_CHECK_CLOSE(hxp, divcurl_value_at(tx + 1.0 / h, ty), 1);
       BOOST_CHECK_CLOSE(hxm, divcurl_value_at(tx, ty - 1.0 / h), 1);
       BOOST_CHECK_CLOSE(hxp, divcurl_value_at(tx, ty + 1.0 / h), 1);
       BOOST_CHECK_CLOSE( 2.0 * h0 * ( hyp - hym) / (2.0 / h),
                          divcurl_derivative_at(tx, ty).x, 1);
       BOOST_CHECK_CLOSE( 2.0 * h0 * ( hxp - hxm) / (2.0 / h),
                          divcurl_derivative_at(tx, ty).y, 1);
       C2DFVector sv = divcurl.derivative_at(field, 16, 16);
       C2DFVector tv = divcurl_derivative_at(0, 0);
       BOOST_CHECK_CLOSE(h * h * h * sv.x + 1.0, tv.x + 1.0, 1);
       BOOST_CHECK_CLOSE(h * h * h * sv.y + 1.0, tv.y + 1.0, 1);
       sv = divcurl.derivative_at(field, 24, 24);
       tv = divcurl_derivative_at(tx, ty);
       BOOST_CHECK_CLOSE(h * h * h * h * h * sv.x, tv.x, 0.1);
       BOOST_CHECK_CLOSE(h * h * h * h * h * sv.y, tv.y, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_divergence_at_2, TransformSplineFixtureexpm2Field )
{
       size_t nx = 32;
       init(nx, 0.1, ip_bspline3);
       const int vx = 23;
       const int vy = 20;
       const double testvalue = 6.0 * M_PI;
       const double dx = 1.0 / h;
       C2DDivCurlMatrix divcurl(ipf->get_kernel());
       double tx = ( vx - nx / 2 ) * dx;
       double ty = ( vy - nx / 2 ) * dx;
       cvdebug() << "h=" << h
                 << ", tx=" << tx
                 << ", ty=" << ty
                 << "\n";
       // evaluated using maxima
       double h0  = h * h * divcurl.value_at(field, vx, vy);
       double hxp = h * h * divcurl.value_at(field, vx + 1, vy);
       double hxm = h * h * divcurl.value_at(field, vx - 1, vy);
       double hyp = h * h * divcurl.value_at(field, vx, vy + 1);
       double hym = h * h * divcurl.value_at(field, vx, vy - 1);
       BOOST_CHECK_CLOSE( h0, divcurl_value_at(tx, ty), 1);
       BOOST_CHECK_CLOSE(hxm, divcurl_value_at(tx - dx, ty), 1);
       BOOST_CHECK_CLOSE(hxp, divcurl_value_at(tx + dx, ty), 1);
       BOOST_CHECK_CLOSE(hxm, divcurl_value_at(tx, ty - dx), 1);
       BOOST_CHECK_CLOSE(hxp, divcurl_value_at(tx, ty + dx), 1);
       C2DFVector vt = divcurl_derivative_at(tx, ty);
       BOOST_CHECK_CLOSE( h0 * ( hxp - hxm) / dx, vt.x, 1);
       BOOST_CHECK_CLOSE( h0 * ( hyp - hym) / dx, vt.y, 1);
       // derivative scale = pow(h,5)
       C2DFVector sv = h * h * h * h * h * divcurl.derivative_at(field, nx / 2, nx / 2);
       C2DFVector tv = divcurl_derivative_at(0, 0);
       BOOST_CHECK_CLOSE(sv.x + 1.0, tv.x + 1.0, 1);
       BOOST_CHECK_CLOSE(sv.y + 1.0, tv.y + 1.0, 1);
       sv = h * h * h * h * h * divcurl.derivative_at(field, vx, vy);
       tv = divcurl_derivative_at(tx, ty);
       BOOST_CHECK_CLOSE(sv.x, tv.x, 0.1);
       BOOST_CHECK_CLOSE(sv.y, tv.y, 0.1);
}
#endif

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2, TransformSplineFixtureexpm2Field )
{
       init(32, 8.0, ip_bspline3);
       // evaluated using maxima
       const double testvalue = 6.0 * M_PI;
       C2DDivCurlMatrix divcurl(field.get_size(), ipf->get_kernel());
       // this test is just to compare the maxima value to a approximate integration
       float manual = integrate_divcurl(-8, 8, -8, 8, 127, 127);
       BOOST_CHECK_CLOSE(manual, testvalue, 0.1);
       float spline = divcurl.multiply(field);
       BOOST_CHECK_CLOSE(spline, testvalue,  3.0);
       cvdebug() << ""  << testvalue / spline
                 << " sqrt= " << sqrt(testvalue / spline) << "\n";
}


float TransformSplineFixtureexpm2Field::fx(float x, float y)
{
       return exp(-x * x - y * y);
}

float TransformSplineFixtureexpm2Field::fy(float x, float y)
{
       return exp(-x * x - y * y);
}

float TransformSplineFixtureexpm2Field::dfx_xx(float x, float y)
{
       return (4 * x * x  - 2) * fx(x, y);
}


float TransformSplineFixtureexpm2Field::dfx_xy(float x, float y)
{
       return 4 * x * y * fx(x, y);
}

float TransformSplineFixtureexpm2Field::dfx_yy(float x, float y)
{
       return (4 * y * y  - 2) * fx(x, y);
}

float TransformSplineFixtureexpm2Field::dfx_xxx(float x, float y)
{
       return -4 * x * (2 * x * x  - 3) * fx(x, y);
}


float TransformSplineFixtureexpm2Field::dfx_xxy(float x, float y)
{
       return - 4 * y * ( 2 * x * x  - 1)  * fx(x, y);
}

float TransformSplineFixtureexpm2Field::dfx_xyy(float x, float y)
{
       return - 4 * x * (2 * y * y  - 1) * fx(x, y);
}


float TransformSplineFixtureexpm2Field::dfx_yyy(float x, float y)
{
       return -4 * y * (2 * y * y  - 3) * fx(x, y);
}


float TransformSplineFixtureexpm2Field::dfy_xxx(float x, float y)
{
       return -4 * x * (2 * x * x  - 3) * fy(x, y);
}


float TransformSplineFixtureexpm2Field::dfy_xxy(float x, float y)
{
       return - 4 * y * ( 2 * x * x  - 1)  * fy(x, y);
}

float TransformSplineFixtureexpm2Field::dfy_xyy(float x, float y)
{
       return - 4 * x * (2 * y * y  - 1) * fy(x, y);
}


float TransformSplineFixtureexpm2Field::dfy_yyy(float x, float y)
{
       return -4 * y * (2 * y * y  - 3) * fy(x, y);
}

float TransformSplineFixtureexpm2Field::dfy_xx(float x, float y)
{
       return (4 * x * x  - 2) * fy(x, y);
}

float TransformSplineFixtureexpm2Field::dfy_xy(float x, float y)
{
       return 4 * x * y * fy(x, y);
}

float TransformSplineFixtureexpm2Field::dfy_yy(float x, float y)
{
       return (4 * y * y  - 2) * fy(x, y);
}

double TransformSplineFixtureexpm2Field::divcurl_value_at(float x, float y)
{
       double vfx_xx = dfx_xx(x, y);
       double vfx_xy = dfx_xy(x, y);
       double vfy_xy = dfy_xy(x, y);
       double vfy_yy = dfy_yy(x, y);
       return  vfx_xx  + vfx_xy  + vfy_xy + vfy_yy;
}

C2DFVector TransformSplineFixtureexpm2Field::divcurl_derivative_at(float x, float y)
{
       double v = divcurl_value_at(x, y);
       return 2 * v * C2DFVector(
                     dfx_xxx(x, y) + dfx_xxy(x, y)  + dfx_xxy(x, y) + dfx_xyy(x, y),
                     dfy_xxy(x, y) + dfy_xyy(x, y)  + dfy_xyy(x, y) + dfy_yyy(x, y));
}


float TransformSplineFixtureexpm2Field::integrate_divcurl(float x1, float x2,
              float y1, float y2, int xinterv, int yinterv)
{
       float hx = (x2 - x1) / xinterv;
       float hy = (y2 - y1) / yinterv;
       double sum = 0.0;

       for (float y = y1 + hx / 2.0; y < y2; y += hy)
              for (float x = x1 + hx / 2.0; x < x2; x += hx) {
                     double v = divcurl_value_at(x, y);
                     sum += v * v;
              }

       sum *= hx * hy;
       return sum;
}

