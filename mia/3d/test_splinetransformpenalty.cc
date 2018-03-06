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
#include <mia/internal/pluginsettest.hh>
#include <mia/3d/splinetransformpenalty.hh>

NS_MIA_USE;

class C3DSplinePenaltyMock: public C3DSplineTransformPenalty
{
public:

       C3DSplinePenaltyMock(double weight, bool normalize);

private:
       void do_initialize();

       double do_value(const C3DFVectorfield&  m_coefficients) const;

       double do_value_and_gradient(const C3DFVectorfield&  m_coefficients, CDoubleVector& gradient) const;

       C3DSplineTransformPenalty *do_clone() const;

       C3DFVector m_scale;
};


BOOST_AUTO_TEST_CASE( weight_1_1_1_1p5_2p9_bspline3 )
{
       C3DSplinePenaltyMock penalty(1.0, false);
       C3DBounds size(1, 1, 1);
       penalty.initialize(size, C3DFVector(1.5, 2.9, 2.0), produce_spline_kernel("bspline:d=2"));
       C3DFVectorfield coef(size);
       coef(0, 0, 0) = C3DFVector(1.0 / 1.5, 1.0 / 2.9, 1.0 / 2.0);
       BOOST_CHECK_CLOSE(penalty.value(coef), 3.0, 0.1);
       CDoubleVector grad(3);
       BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 3.0, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 1.0, 0.1);
       std::unique_ptr<C3DSplineTransformPenalty> penalty2(penalty.clone());
       BOOST_CHECK_CLOSE(penalty2->value(coef), 3.0, 0.1);
       BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 3.0, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 1.0, 0.1);
}

BOOST_AUTO_TEST_CASE( weight_2_2_2_1p5_2p9_bspline3_normalize )
{
       C3DSplinePenaltyMock penalty(1.0, true);
       C3DBounds size(2, 2, 2);
       penalty.initialize(size, C3DFVector(2.0, 2.0, 2.0), produce_spline_kernel("bspline:d=2"));
       C3DFVectorfield coef(size);
       coef(0, 0, 0) = C3DFVector(1.0 / 2.0, 1.0, 1.0 / 4.0);
       BOOST_CHECK_CLOSE(penalty.value(coef), 1.75 / 8.0, 0.1);
       CDoubleVector grad(24);
       BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 1.75 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 0.5 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 1.0 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 0.25 / 8.0, 0.1);
       std::unique_ptr<C3DSplineTransformPenalty> penalty2(penalty.clone());
       BOOST_CHECK_CLOSE(penalty2->value(coef), 1.75 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 1.75 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 0.5 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 1.0 / 8.0, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 0.25 / 8.0, 0.1);
}


BOOST_AUTO_TEST_CASE( weight_0p5_1_1_1p5_2p9_bspline3 )
{
       C3DSplinePenaltyMock penalty(0.5, false);
       C3DBounds size(1, 1, 1);
       penalty.initialize(size, C3DFVector(1.5, 2.9, 2.0), produce_spline_kernel("bspline:d=2"));
       C3DFVectorfield coef(size);
       coef(0, 0, 0) = C3DFVector(1.0 / 1.5, 1.0 / 2.9, 1.0 / 2.0);
       BOOST_CHECK_CLOSE(penalty.value(coef), 1.5, 0.1);
       CDoubleVector grad(3);
       BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 1.5, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 0.5, 0.1);
       std::unique_ptr<C3DSplineTransformPenalty> penalty2(penalty.clone());
       BOOST_CHECK_CLOSE(penalty2->value(coef), 1.5, 0.1);
       BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 1.5, 0.1);
       BOOST_CHECK_CLOSE(grad[0], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[1], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[2], 0.5, 0.1);
}

BOOST_AUTO_TEST_CASE( weight_0p5_2_3_2_3_bspline3 )
{
       C3DSplinePenaltyMock penalty(0.5, false);
       C3DBounds size(2, 3, 1);
       penalty.initialize(size, C3DFVector(2, 3, 1), produce_spline_kernel("bspline:d=2"));
       C3DFVectorfield coef(size);
       auto ic = coef.begin_range(C3DBounds::_0, size);
       auto ec = coef.end_range(C3DBounds::_0, size);

       while (ic != ec) {
              *ic = ic.pos();
              ++ic;
       }

       BOOST_CHECK_CLOSE(penalty.value(coef), 4.5, 0.1);
       CDoubleVector grad(18);
       BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 4.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 0], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 1], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 2], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 3], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 4], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 5], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 6], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 7], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 8], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 9], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[10], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[11], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[12], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[13], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[14], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[15], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[16], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[17], 0.0, 0.1);
       std::unique_ptr<C3DSplineTransformPenalty> penalty2(penalty.clone());
       BOOST_CHECK_CLOSE(penalty2->value(coef), 4.5, 0.1);
       BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 4.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 0], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 1], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 2], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 3], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 4], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 5], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 6], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 7], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[ 8], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[ 9], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[10], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[11], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[12], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[13], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[14], 0.0, 0.1);
       BOOST_CHECK_CLOSE(grad[15], 0.5, 0.1);
       BOOST_CHECK_CLOSE(grad[16], 1.0, 0.1);
       BOOST_CHECK_CLOSE(grad[17], 0.0, 0.1);
}

C3DSplinePenaltyMock::C3DSplinePenaltyMock(double weight, bool normalize):
       C3DSplineTransformPenalty(weight, normalize)
{
}

void C3DSplinePenaltyMock::do_initialize()
{
       m_scale.x = get_range().x / get_size().x;
       m_scale.y = get_range().y / get_size().y;
       m_scale.z = get_range().z / get_size().z;
}

double C3DSplinePenaltyMock::do_value(const C3DFVectorfield&  coefficients) const
{
       double result = 0.0;
       auto ic = coefficients.begin();
       auto ec = coefficients.end();

       while (ic != ec) {
              result += m_scale.x * ic->x;
              result += m_scale.y * ic->y;
              result += m_scale.z * ic->z;
              ++ic;
       }

       return result;
}


double C3DSplinePenaltyMock::do_value_and_gradient(const C3DFVectorfield&  coefficients, CDoubleVector& gradient) const
{
       double result = 0.0;
       auto ic = coefficients.begin();
       auto ec = coefficients.end();
       auto ig = gradient.begin();

       while (ic != ec) {
              *ig = -m_scale.x * ic->x;
              result += *ig++;
              *ig = -m_scale.y * ic->y;
              result += *ig++;
              *ig = -m_scale.z * ic->z;
              result += *ig++;
              ++ic;
       }

       return -result;
}

C3DSplineTransformPenalty *C3DSplinePenaltyMock::do_clone() const
{
       C3DSplineTransformPenalty *result =  new C3DSplinePenaltyMock(get_weight(), get_normalize());
       result->initialize(get_size(), get_range(), get_kernel());
       return result;
}


BOOST_AUTO_TEST_CASE(test_available_plugins_spline_penalty)
{
       std::set<std::string> test_data = {"divcurl"};
       test_available_plugins(C3DSplineTransformPenaltyPluginHandler::instance(), test_data);
}
