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

#include <mia/2d/ppmatrix.hh>

NS_MIA_USE;

using std::string;

struct TransformSplineFixtureFieldBase {
       TransformSplineFixtureFieldBase()
       {
       }
       void init(int dsize, float range, const string& kernel)
       {
              init2d(T2DVector<int>(dsize, dsize), range, kernel);
       }
       void init2d(const T2DVector<int>& dsize, float range, const std::string& kernel)
       {
              ipf.reset(new C2DInterpolatorFactory(kernel, "mirror"));
              size = C2DBounds(2 * dsize.x + 1, 2 * dsize.y + 1);
              field = C2DFVectorfield(size);
              scale.x = range / dsize.x;
              scale.y = range / dsize.y;
              h.x = dsize.x / range;
              h.y = dsize.y / range;
              field_range.x = 2 * range;
              field_range.y = 2 * range;
              C2DFVectorfield::iterator i = field.begin();

              for (int y = -dsize.y; y <= dsize.y; ++y)
                     for (int x = -dsize.x; x <= dsize.x; ++x, ++i) {
                            float sx = x * scale.x;
                            float sy = y * scale.y;
                            *i = C2DFVector( fx(sx, sy), fy(sx, sy));
                     }

              source.reset(ipf->create(field));
       }
       C2DBounds size;
       C2DFVectorfield field;
       C2DFVector h;
       P2DInterpolatorFactory ipf;
       C2DBounds field_range;
       std::shared_ptr<T2DInterpolator<C2DFVector>> source;
protected:
       virtual float fx(float x, float y) = 0;
       virtual float fy(float x, float y) = 0;
       C2DFVector scale;
private:

};



struct TransformSplineFixtureConst: public TransformSplineFixtureFieldBase {
       void prepare(int dsize, float range, const std::string& kernel, float fx, float fy);
       virtual float fx(float x, float y);
       virtual float fy(float x, float y);
private:
       C2DFVector m_f;
};

void TransformSplineFixtureConst::prepare(int dsize, float range, const std::string& kernel, float fx, float fy)
{
       m_f.x = fx;
       m_f.y = fy;
       init(dsize, range, kernel);
}

float TransformSplineFixtureConst::fx(float, float )
{
       return m_f.x;
}

float TransformSplineFixtureConst::fy(float, float )
{
       return m_f.y;
}


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

       float integrate_div(float x1, float x2, float y1, float y2, int xinterv, int yinterv);
       double div_value_at(float x, float y);
       C2DFVector div_derivative_at(float x, float y);
};

struct TransformSplineFixtureDivOnly: public TransformSplineFixtureFieldBase {
       float fx(float x, float y);
       float fy(float x, float y);
};

struct TransformSplineFixtureCurlOnly: public TransformSplineFixtureFieldBase {
       float fx(float x, float y);
       float fy(float x, float y);
};


struct TransformSplineFixtureexpm2Field_44: public TransformSplineFixtureexpm2Field {
       void run(int dsize, float range, const string& type, double corr = 1.0);
};

struct TransformSplineFixtureexpm2testInterp : public TransformSplineFixtureexpm2Field {
       void run(int dsize, float range, const string& type);
};


BOOST_FIXTURE_TEST_CASE( test_nocurl_bspline3, TransformSplineFixtureDivOnly )
{
       init(16, 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       const double testvalue = 6.0 * M_PI;
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
       BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 0.1);
}

BOOST_FIXTURE_TEST_CASE( test_nocurl_bspline3_noniso, TransformSplineFixtureDivOnly )
{
       init2d(T2DVector<int>(32, 14), 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       const double testvalue = 6.0 * M_PI;
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
       BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_nocurl_bspline3_8_4, TransformSplineFixtureDivOnly )
{
       init(8, 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       const double testvalue = 6.0 * M_PI;
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       BOOST_CHECK_CLOSE( div  * coeffs, testvalue, 0.5);
       C2DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
       BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.5);
       C2DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       BOOST_CHECK_CLOSE( 1.0 + rot * coeffs, 1.0, 0.3);
}

BOOST_FIXTURE_TEST_CASE( test_nodiv_bspline3, TransformSplineFixtureCurlOnly )
{
       init(16, 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       const double testvalue = 6.0 * M_PI;
       C2DPPDivcurlMatrix curl(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       BOOST_CHECK_CLOSE( curl  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix divcurl(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 1.0);
       BOOST_CHECK_CLOSE( divcurl  * coeffs, testvalue, 0.1);
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       BOOST_CHECK_CLOSE( 1.0 + div * coeffs, 1.0, 0.1);
}


// test whether the interpolation is "good enough"
BOOST_FIXTURE_TEST_CASE( test_interpolation_16_2_bspline3, TransformSplineFixtureexpm2testInterp )
{
       run( 8, 2.0, "bspline:d=3");
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_16_8, TransformSplineFixtureexpm2Field_44 )
{
       run(16, 4, "bspline:d=3", 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_8_8, TransformSplineFixtureexpm2Field_44 )
{
       run(8, 4, "bspline:d=3", 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_grad, TransformSplineFixtureexpm2Field_44 )
{
       init(8, 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       CDoubleVector gradient(field.size() * 2, true);
       div.evaluate(coeffs, gradient);
       auto ig = gradient.begin();
       auto ic = coeffs.begin();

       for (size_t y = 0; y < field.get_size().y; ++y)
              for (size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
                     ic->x += 0.001;
                     double graddivp = div * coeffs;
                     ic->x -= 0.002;
                     double graddivm = div * coeffs;
                     ic->x += 0.001;
                     double test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (x)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[0], test_grad, 2);

                     ic->y += 0.001;
                     graddivp = div * coeffs;
                     ic->y -= 0.002;
                     graddivm = div * coeffs;
                     ic->y += 0.001;
                     test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (y)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[1], test_grad, 2);
              }
}

BOOST_FIXTURE_TEST_CASE( test_rotation_expm2_bspline3_grad, TransformSplineFixtureexpm2Field_44 )
{
       init(8, 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       C2DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       CDoubleVector gradient(field.size() * 2, true);
       rot.evaluate(coeffs, gradient);
       auto ig = gradient.begin();
       auto ic = coeffs.begin();

       for (size_t y = 0; y < field.get_size().y; ++y)
              for (size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
                     ic->x += 0.001;
                     double graddivp = rot * coeffs;
                     ic->x -= 0.002;
                     double graddivm = rot * coeffs;
                     ic->x += 0.001;
                     double test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (x)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[0], test_grad, 2);

                     ic->y += 0.001;
                     graddivp = rot * coeffs;
                     ic->y -= 0.002;
                     graddivm = rot * coeffs;
                     ic->y += 0.001;
                     test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (y)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[1], test_grad, 2);
              }
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline3_grad_noiso, TransformSplineFixtureexpm2Field_44 )
{
       init2d(T2DVector<int>(12, 9), 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       CDoubleVector gradient(field.size() * 2, true);
       div.evaluate(coeffs, gradient);
       auto ig = gradient.begin();
       auto ic = coeffs.begin();

       for (size_t y = 0; y < field.get_size().y; ++y)
              for (size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
                     ic->x += 0.001;
                     double graddivp = div * coeffs;
                     ic->x -= 0.002;
                     double graddivm = div * coeffs;
                     ic->x += 0.001;
                     double test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (x)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[0], test_grad, 1);

                     ic->y += 0.001;
                     graddivp = div * coeffs;
                     ic->y -= 0.002;
                     graddivm = div * coeffs;
                     ic->y += 0.001;
                     test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (y)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[1], test_grad, 1);
              }
}

BOOST_FIXTURE_TEST_CASE( test_rotation_expm2_bspline3_grad_noiso, TransformSplineFixtureexpm2Field_44 )
{
       init2d(T2DVector<int>(12, 9), 4, "bspline:d=4");
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       C2DPPDivcurlMatrix rot(field.get_size(), field_range, *ipf->get_kernel(), 0.0, 1.0);
       CDoubleVector gradient(field.size() * 2, true);
       rot.evaluate(coeffs, gradient);
       auto ig = gradient.begin();
       auto ic = coeffs.begin();

       for (size_t y = 0; y < field.get_size().y; ++y)
              for (size_t x = 0; x < field.get_size().x; ++x, ig += 2, ++ic) {
                     ic->x += 0.001;
                     double graddivp = rot * coeffs;
                     ic->x -= 0.002;
                     double graddivm = rot * coeffs;
                     ic->x += 0.001;
                     double test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (x)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[0], test_grad, 1);

                     ic->y += 0.001;
                     graddivp = rot * coeffs;
                     ic->y -= 0.002;
                     graddivm = rot * coeffs;
                     ic->y += 0.001;
                     test_grad = (graddivp - graddivm) / 0.002;
                     cvdebug() << x << " " << y << " (y)\n";

                     if (fabs(test_grad) > 0.0001)
                            BOOST_CHECK_CLOSE(ig[1], test_grad, 1);
              }
}


BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline4, TransformSplineFixtureexpm2Field_44 )
{
       run(16, 4, "bspline:d=4", 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_divergence_expm2_bspline4_8_4, TransformSplineFixtureexpm2Field_44 )
{
       run(8, 4, "bspline:d=4", 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_divergence_zero_x, TransformSplineFixtureConst )
{
       prepare(16, 16, "bspline:d=4", 0, .01);
       const double testvalue = 0.0;
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       float spline = div * coeffs;
       BOOST_CHECK_CLOSE(spline + 1.0, testvalue + 1.0,  1);
       BOOST_CHECK_EQUAL(field.get_size(), div.get_size());
}


float TransformSplineFixtureDivOnly::fx(float x, float y)
{
       return x * exp(-x * x - y * y);
}

float TransformSplineFixtureDivOnly::fy(float x, float y)
{
       return y * exp(-x * x - y * y);
}


float TransformSplineFixtureCurlOnly::fx(float x, float y)
{
       return y * exp(-x * x - y * y);
}

float TransformSplineFixtureCurlOnly::fy(float x, float y)
{
       return -x * exp(-x * x - y * y);
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

double TransformSplineFixtureexpm2Field::div_value_at(float x, float y)
{
       const double vfx = dfx_xx(x, y) + dfy_xy(x, y);
       const double vfy = dfx_xy(x, y) + dfy_yy(x, y);
       return   vfx * vfx + vfy * vfy;
}

C2DFVector TransformSplineFixtureexpm2Field::div_derivative_at(float x, float y)
{
       double fy_yyy = dfy_yyy(x, y);
       double fy_xyy = dfy_xyy(x, y);
       double fx_xyy = dfx_xyy(x, y);
       double fx_xxy = dfx_xxy(x, y);
       double fy_xxy = dfy_xxy(x, y);
       double fx_xxx = dfx_xxx(x, y);
       double fy_yy =  dfy_yy(x, y);
       double fx_xx =  dfx_xx(x, y);
       double fx_xy =  dfx_xy(x, y);
       double fy_xy =  dfy_xy(x, y);
       return C2DFVector(
                     2 * (( fy_xyy + fx_xxy ) * ( fy_yy + fx_xy) +
                          ( fy_xy + fx_xx) * (fy_xxy + fx_xxx)),
                     2 * (( fy_yy + fx_xy) * (fy_yyy + fx_xyy) +
                          ( fy_xy + fx_xx) * (fy_xyy + fx_xxy))
              );
}


float TransformSplineFixtureexpm2Field::integrate_div(float x1, float x2,
              float y1, float y2, int xinterv, int yinterv)
{
       float hx = (x2 - x1) / xinterv;
       float hy = (y2 - y1) / yinterv;
       double sum = 0.0;

       for (float y = y1 + hx / 2.0; y < y2; y += hy)
              for (float x = x1 + hx / 2.0; x < x2; x += hx)
                     sum += div_value_at(x, y);

       sum *= hx * hy;
       return sum;
}



void TransformSplineFixtureexpm2testInterp::run(int size, float range, const string& kernel)
{
       init( size, range, kernel);

       for (float y = -range; y < range; y += range / 5.0)
              for (float x = -range; x < range; x += range / 5.0) {
                     C2DFVector hx(x * h.x + size, y * h.y + size);
                     C2DFVector hy = (*source)(hx);
                     BOOST_CHECK_CLOSE(1.0 + hy.x, 1.0 + fx(x, y ), 0.3);
                     BOOST_CHECK_CLOSE(1.0 + hy.y, 1.0 + fy(x, y ), 0.3);
              }
}

void TransformSplineFixtureexpm2Field_44::run(int dsize, float range, const string& type, double corr)
{
       init(dsize, range, type);
       // evaluated using maxima
       const double testvalue = 4.0 * M_PI;
       const T2DInterpolator<C2DFVector>& interp =
              static_cast<const T2DInterpolator<C2DFVector>&>(*source);
       auto coeffs = interp.get_coefficients();
       // this test is just to compare the maxima value to a approximate integration
       float manual = integrate_div(-range, range, -range, range, 63, 63);
       BOOST_CHECK_CLOSE(manual, testvalue, 1);
       C2DPPDivcurlMatrix div(field.get_size(), field_range, *ipf->get_kernel(), 1.0, 0.0);
       float spline = corr * (div * coeffs);
       cvmsg() << "(" << dsize << "," << range <<  ") " << spline << ", ratio=" << manual / spline << "\n";
       BOOST_CHECK_CLOSE(spline, manual,  1);
}
