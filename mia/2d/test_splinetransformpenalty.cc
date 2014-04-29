/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/2d/splinetransformpenalty.hh>

NS_MIA_USE; 

PrepareTestPluginPath plugin_path_init; 

class C2DSplinePenaltyMock: public C2DSplineTransformPenalty {
public: 
	
	C2DSplinePenaltyMock(double weight, bool normalize); 
	
private: 
	void do_initialize(); 
	
	double do_value(const C2DFVectorfield&  m_coefficients) const; 
	
	double do_value_and_gradient(const C2DFVectorfield&  m_coefficients, CDoubleVector& gradient) const;
	
	C2DSplineTransformPenalty *do_clone() const;

	C2DFVector m_scale; 
}; 


BOOST_AUTO_TEST_CASE( weight_1_1_1_1p5_2p9_bspline3 )
{
	C2DSplinePenaltyMock penalty(1.0, false); 
	C2DBounds size(1,1); 
	penalty.initialize(size, C2DFVector(1.5,2.9), produce_spline_kernel("bspline:d=2"));

	C2DFVectorfield coef(size); 
	coef(0,0) = C2DFVector(1.0/1.5,1.0/2.9); 
	
	BOOST_CHECK_CLOSE(penalty.value(coef), 2.0, 0.1); 

	CDoubleVector grad(2); 
	BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 2.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[0], 1.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[1], 1.0, 0.1); 

	std::unique_ptr<C2DSplineTransformPenalty> penalty2(penalty.clone()); 
	
	BOOST_CHECK_CLOSE(penalty2->value(coef), 2.0, 0.1); 
	BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 2.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[0], 1.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[1], 1.0, 0.1); 
	
}


BOOST_AUTO_TEST_CASE( weight_0p5_1_1_1p5_2p9_bspline3 )
{
	C2DSplinePenaltyMock penalty(0.5, false); 
	C2DBounds size(1,1); 
	penalty.initialize(size, C2DFVector(1.5,2.9), produce_spline_kernel("bspline:d=2"));

	C2DFVectorfield coef(size); 
	coef(0,0) = C2DFVector(1.0/1.5,1.0/2.9); 
	
	BOOST_CHECK_CLOSE(penalty.value(coef), 1.0, 0.1); 

	CDoubleVector grad(2); 
	BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 1.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[0], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[1], 0.5, 0.1); 

	std::unique_ptr<C2DSplineTransformPenalty> penalty2(penalty.clone()); 
	
	BOOST_CHECK_CLOSE(penalty2->value(coef), 1.0, 0.1); 
	BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 1.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[0], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[1], 0.5, 0.1); 
	
}

BOOST_AUTO_TEST_CASE( weight_0p5_2_3_2_3_bspline3 )
{
	C2DSplinePenaltyMock penalty(0.5, false); 
	C2DBounds size(2,3); 
	penalty.initialize(size, C2DFVector(2,3), produce_spline_kernel("bspline:d=2"));

	C2DFVectorfield coef(size); 
	auto ic = coef.begin_range(C2DBounds::_0, size);
	auto ec = coef.end_range(C2DBounds::_0, size);
	
	while (ic != ec) {
		*ic = ic.pos(); 
		++ic; 
	}
	
	BOOST_CHECK_CLOSE(penalty.value(coef), 4.5, 0.1); 

	CDoubleVector grad(12); 
	BOOST_CHECK_CLOSE(penalty.value_and_gradient(coef, grad), 4.5, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 0], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 1], 0.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 2], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 3], 0.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 4], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 5], 0.5, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 6], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 7], 0.5, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 8], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 9], 1.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[10], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[11], 1.0, 0.1); 


	std::unique_ptr<C2DSplineTransformPenalty> penalty2(penalty.clone()); 
	
	BOOST_CHECK_CLOSE(penalty2->value(coef), 4.5, 0.1); 
	BOOST_CHECK_CLOSE(penalty2->value_and_gradient(coef, grad), 4.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 0], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 1], 0.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 2], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 3], 0.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 4], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 5], 0.5, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 6], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 7], 0.5, 0.1); 

	BOOST_CHECK_CLOSE(grad[ 8], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(grad[ 9], 1.0, 0.1); 

	BOOST_CHECK_CLOSE(grad[10], 0.5, 0.1); 
	BOOST_CHECK_CLOSE(grad[11], 1.0, 0.1); 	
}

C2DSplinePenaltyMock::C2DSplinePenaltyMock(double weight, bool normalize):
C2DSplineTransformPenalty(weight,normalize)
{
}

void C2DSplinePenaltyMock::do_initialize()
{
	m_scale.x = get_range().x / get_size().x; 
	m_scale.y = get_range().y / get_size().y; 
}

double C2DSplinePenaltyMock::do_value(const C2DFVectorfield&  coefficients) const
{
	double result = 0.0; 
	auto ic = coefficients.begin(); 
	auto ec = coefficients.end(); 

	while (ic != ec) {
		result += m_scale.x * ic->x; 
		result += m_scale.y * ic->y;
		++ic; 
	}
	return result; 
}


double C2DSplinePenaltyMock::do_value_and_gradient(const C2DFVectorfield&  coefficients, CDoubleVector& gradient) const
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
		++ic; 
	}
	return -result; 
}

C2DSplineTransformPenalty *C2DSplinePenaltyMock::do_clone() const
{
	C2DSplineTransformPenalty *result =  new C2DSplinePenaltyMock(get_weight(), get_normalize());
	result->initialize(get_size(), get_range(), get_kernel()); 
	return result; 
}
  

CSplineKernelTestPath kernel_test_path; 


C2DSplineTransformPenaltyPluginHandlerTest penalty_plug_path; 

BOOST_AUTO_TEST_CASE(test_available_plugins)
{
	std::set<std::string> test_data = {"divcurl"}; 
	test_availabe_plugins(C2DSplineTransformPenaltyPluginHandler::instance(), test_data); 
}
