/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "TRANSFORMS"

#include <ostream>
#include <fstream>
#include <mia/2d/transform.hh>
#include <mia/internal/autotest.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/transformmock.hh>
#include <mia/2d/transform/spline.hh>

NS_MIA_USE; 
namespace bfs=boost::filesystem;

CSplineKernelTestPath spline_kernel_path_init; 

class Cost2DMock {
public: 
	Cost2DMock(const C2DBounds& size); 
	double value(const C2DTransformation& t) const;  
	double value_and_gradient(C2DFVectorfield& gradient) const;
	
	double src_value(const C2DFVector& x)const; 
	double ref_value(const C2DFVector& x)const; 
	C2DFVector src_grad(const C2DFVector& x)const; 
	C2DBounds m_size; 
	C2DFVector m_center; 
	float m_r; 
}; 

class TransformGradientFixture {
public: 
	TransformGradientFixture(); 

	void run_test(C2DTransformation& t, double tol=0.1)const; 

	C2DBounds size; 
	Cost2DMock cost; 

	C2DFVector x; 
	C2DFVectorfield gradient; 
}; 



BOOST_AUTO_TEST_CASE (selftest_Cost2DMock) 
{
	C2DBounds size(20,30); 
	Cost2DMock cm(size); 
	
	C2DFVector x(11.0,16.0); 
	C2DFVector dx(.001,0.0); 
	C2DFVector dy(.0,0.001); 
	
	C2DFVector grad = cm.src_grad(x);
	
	C2DFVector fdgrad( (cm.src_value(x + dx) - cm.src_value(x - dx)) / 0.002, 
			   (cm.src_value(x + dy) - cm.src_value(x - dy)) / 0.002); 
	
	BOOST_CHECK_CLOSE(grad.x , fdgrad.x, 0.1); 
	BOOST_CHECK_CLOSE(grad.y , fdgrad.y, 0.1); 
	
}

BOOST_FIXTURE_TEST_CASE (test_translate_Gradient, TransformGradientFixture) 
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory creater = handler.produce("translate");
	P2DTransformation transform = creater->create(size);

	run_test(*transform); 
	

}

BOOST_FIXTURE_TEST_CASE (test_rigid_Gradient, TransformGradientFixture) 
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory creater = handler.produce("rigid");
	P2DTransformation transform = creater->create(size);

	run_test(*transform); 
	

}

BOOST_FIXTURE_TEST_CASE (test_affine_Gradient, TransformGradientFixture) 
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory creater = handler.produce("affine");
	P2DTransformation transform = creater->create(size);

	run_test(*transform); 
}

BOOST_FIXTURE_TEST_CASE (test_vf_Gradient, TransformGradientFixture) 
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory creater = handler.produce("vf");
	P2DTransformation transform = creater->create(size);

	run_test(*transform, 2.0); 
}


BOOST_FIXTURE_TEST_CASE (test_spline_Gradient, TransformGradientFixture) 
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory creater = handler.produce("spline:rate=2");
	P2DTransformation transform = creater->create(size);

	// this is a quite high tolerance, but with all the interpolation going 
	// on the evaluation is quite sensible to small changes and finite 
	// differences are not very accurate 
	run_test(*transform, 16.0); 
}

BOOST_FIXTURE_TEST_CASE (test_transform_iterator, TransformGradientFixture) 
{
	C2DBounds size(3,5); 
	C2DTransformMock t(size);

	auto it = t.begin(); 
	for(size_t i = 0; i < size.x * size.y; ++i, ++it);
	BOOST_CHECK_EQUAL(it, t.end()); 

	auto it2 = t.begin();
	
	++it2; 
	it2 += 5; 
	
	auto itt = t.begin(); 
	for(int i = 0; i < 6; ++i, ++itt); 
	
	BOOST_CHECK_EQUAL(it2, itt); 

	BOOST_CHECK_EQUAL(t.begin() + 6, itt); 

	auto it3 = t.begin(); 
	it3 += size.x * size.y; 
	
	BOOST_CHECK_EQUAL(it3, t.end()); 
}



TransformGradientFixture::TransformGradientFixture():
	size(20,30), 
	cost(size),
	x(11,16), 
	gradient(size)

{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);

	cost.value_and_gradient(gradient);
	
}

void TransformGradientFixture::run_test(C2DTransformation& t, double tol)const
{
	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.001; 

	int n_close_zero = 0; 

	int n_zero = 0; 
	for(auto itrg =  trgrad.begin(), 
		    iparam = params.begin(); itrg != trgrad.end(); ++itrg, ++iparam) {
		*iparam += delta; 
		t.set_parameters(params);
		double cost_plus = cost.value(t);
		*iparam -= 2*delta; 
		t.set_parameters(params);
		double cost_minus = cost.value(t);
		*iparam += delta; 
		cvdebug() << cost_plus << ", " << cost_minus << "\n"; 

		double test_val = (cost_plus - cost_minus)/ (2*delta); 
		cvdebug() << *itrg << " vs " << test_val << "\n"; 
		if (fabs(*itrg) < 1e-8 && fabs(test_val) < 1e-8) {
			n_close_zero++; 
			continue; 
		}
		if (*itrg == 0.0 && fabs(test_val) < 1e-7) {
			n_zero++; 
			continue; 
		}

		BOOST_CHECK_CLOSE(*itrg, test_val, tol); 
	}
	cvmsg() << "value pairs < 1e-8 = " << n_close_zero << "\n"; 
	cvmsg() << "grad value zero, but finite difference below 1e-7 = " << n_zero << "\n"; 
}

Cost2DMock::Cost2DMock(const C2DBounds& size):
	m_size(size), 
	m_center(0.5 * size.x, 0.5 * size.y),
	m_r(sqrt(m_center.x * m_center.x + m_center.y * m_center.y))
{
}
	
double Cost2DMock::value(const C2DTransformation& t) const
{
	assert(m_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t y = 0; y < m_size.y; ++y) 
		for (size_t x = 0; x < m_size.x; ++x, ++it) {
			double v = src_value(*it) - ref_value(C2DFVector(x,y)); 
			result += v * v; 
		}
	return result; 
		
}

double Cost2DMock::value_and_gradient(C2DFVectorfield& gradient) const
{
	assert(gradient.get_size() == m_size); 
	
	double result = 0.0; 

	auto ig = gradient.begin(); 
	for (size_t y = 0; y < m_size.y; ++y) 
		for (size_t x = 0; x < m_size.x; ++x, ++ig) {
			C2DFVector pos(x,y);
			double v = src_value(pos) - ref_value(pos); 
			result += v * v; 
			*ig = 2.0 * v * src_grad(pos);  
		}
	return result; 
}

double Cost2DMock::src_value(const C2DFVector& x)const
{
	const C2DFVector p = x - m_center; 
	return exp( - (p.x * p.x + p.y * p.y) / m_r); 
}

C2DFVector Cost2DMock::src_grad(const C2DFVector& x)const
{
	
	return - 2.0f / m_r * (x-m_center) * src_value(x); 
}

double Cost2DMock::ref_value(const C2DFVector& x)const 
{
	const C2DFVector p = x - m_center - C2DFVector(1.0,1.0); 
	return exp( - (p.x * p.x + p.y * p.y) / m_r); 
}




