/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "TRANSFORMS"

#include <ostream>
#include <fstream>
#include <mia/3d/transform.hh>
#include <mia/internal/autotest.hh>
#include <mia/3d/transformfactory.hh>

NS_MIA_USE; 
namespace bfs=boost::filesystem;

PrepareTestPluginPath plugin_path_init; 

class Cost3DMock {
public: 
	Cost3DMock(const C3DBounds& size); 
	double value(const C3DTransformation& t) const;  
	double value_and_gradient(C3DFVectorfield& gradient) const;
	
	double src_value(const C3DFVector& x)const; 
	double ref_value(const C3DFVector& x)const; 
	C3DFVector src_grad(const C3DFVector& x)const; 
	C3DBounds m_size; 
	C3DFVector m_center; 
	float m_r; 
}; 

class TransformGradientFixture {
public: 
	TransformGradientFixture(); 

	void run_test(C3DTransformation& t, double tol=0.1)const; 

	C3DBounds size; 
	Cost3DMock cost; 

	C3DFVector m_x; 
	C3DFVectorfield gradient; 
}; 



BOOST_AUTO_TEST_CASE (selftest_Cost3DMock) 
{
	C3DBounds size(20,30,15); 
	Cost3DMock cm(size); 
	
	C3DFVector x(11.0,16.0, 7.0); 
	C3DFVector dx(.001,0.0,0); 
	C3DFVector dy(.0,0.001,0); 
	C3DFVector dz(.0,.0, 0.001); 
	
	C3DFVector grad = cm.src_grad(x);
	
	C3DFVector fdgrad( (cm.src_value(x + dx) - cm.src_value(x - dx)) / 0.002, 
			   (cm.src_value(x + dy) - cm.src_value(x - dy)) / 0.002,
			   (cm.src_value(x + dz) - cm.src_value(x - dz)) / 0.002
			   ); 
	
	BOOST_CHECK_CLOSE(grad.x , fdgrad.x, 0.1); 
	BOOST_CHECK_CLOSE(grad.y , fdgrad.y, 0.1); 
	BOOST_CHECK_CLOSE(grad.z , fdgrad.z, 0.1); 
	
}

BOOST_FIXTURE_TEST_CASE (test_translate_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("translate");
	P3DTransformation transform = creater->create(size);

	run_test(*transform); 
	

}

BOOST_FIXTURE_TEST_CASE (test_affine_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("affine");
	P3DTransformation transform = creater->create(size);

	run_test(*transform, 0.2); 
}

BOOST_FIXTURE_TEST_CASE (test_rigid_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("rigid");
	P3DTransformation transform = creater->create(size);

	run_test(*transform); 
	
}

BOOST_FIXTURE_TEST_CASE (test_rigid_Gradient_center_shifted, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("rigid:rot-center=[<0.5,0.4,0.6>]");
	P3DTransformation transform = creater->create(size);
	run_test(*transform); 
}


BOOST_FIXTURE_TEST_CASE (test_rotation_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("rotation");
	P3DTransformation transform = creater->create(size);

	run_test(*transform); 
	
}

BOOST_FIXTURE_TEST_CASE (test_rotation_Gradient_center_shifted, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("rotation:rot-center=[<0.5,0.4,0.6>]");
	P3DTransformation transform = creater->create(size);
	run_test(*transform); 
}



#if 0 
BOOST_FIXTURE_TEST_CASE (test_vf_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("vf");
	P3DTransformation transform = creater->create(size);

	run_test(*transform, 2.0); 
}

#endif

#if 0
// this is actually tested in the spline.cc
BOOST_FIXTURE_TEST_CASE (test_spline_Gradient, TransformGradientFixture) 
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory creater = handler.produce("spline:rate=2");
	P3DTransformation transform = creater->create(size);
	transform->set_debug(); 

	run_test(*transform, 10.0); 
}
#endif


struct TransformPluginPathFixture {
	TransformPluginPathFixture(); 
}; 

TransformPluginPathFixture::TransformPluginPathFixture()
{
	CPathNameArray kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C3DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

TransformPluginPathFixture transform_plugin_test_path_init; 

TransformGradientFixture::TransformGradientFixture():
	size(20, 28, 16), 
	cost(size),
	m_x(11,16, 7), 
	gradient(size)

{
	cost.value_and_gradient(gradient);
	
}

void TransformGradientFixture::run_test(C3DTransformation& t, double tol)const
{
	auto params = t.get_parameters();
	CDoubleVector trgrad(params.size()); 
	
	t.translate(gradient,  trgrad); 
	double delta = 0.01; 

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
		if (fabs(*itrg) < 1e-6 && fabs(test_val) < 1e-6) {
			n_close_zero++; 
			continue; 
		}
		if (*itrg == 0.0 && fabs(test_val) < 1e-5) {
			n_zero++; 
			continue; 
		}

		BOOST_CHECK_CLOSE(*itrg, test_val, tol); 
	}
	cvmsg() << "N=" << trgrad.size() << "\n"; 
	cvmsg() << "value pairs < 1e-6 = " << n_close_zero << "\n"; 
	cvmsg() << "grad value zero, but finite difference below 1e-5 = " << n_zero << "\n"; 
}

Cost3DMock::Cost3DMock(const C3DBounds& size):
	m_size(size), 
	m_center(0.5 * size.x, 0.5 * size.y, 0.5 * size.z ),
	m_r(m_center.norm())
{
}
	
double Cost3DMock::value(const C3DTransformation& t) const
{
	assert(m_size == t.get_size()); 
	double result = 0.0; 
	auto it = t.begin(); 
	for (size_t z = 0; z < m_size.z; ++z) 
		for (size_t y = 0; y < m_size.y; ++y) 
			for (size_t x = 0; x < m_size.x; ++x, ++it) {
				double v = src_value(*it) - ref_value(C3DFVector(x,y,z)); 
				result += v * v; 
			}
	return result; 
		
}

double Cost3DMock::value_and_gradient(C3DFVectorfield& gradient) const
{
	assert(gradient.get_size() == m_size); 
	
	double result = 0.0; 

	auto ig = gradient.begin(); 
	for (size_t z = 0; z < m_size.z; ++z) 
		for (size_t y = 0; y < m_size.y; ++y) 
			for (size_t x = 0; x < m_size.x; ++x, ++ig) {
				C3DFVector pos(x,y,z);
				double v = src_value(pos) - ref_value(pos); 
				result += v * v; 
				*ig = 2.0 * v * src_grad(pos);  
			}
	return result; 
}

double Cost3DMock::src_value(const C3DFVector& x)const
{
	const C3DFVector p = x - m_center; 
	return exp( - p.norm2() / m_r  ); 
}

C3DFVector Cost3DMock::src_grad(const C3DFVector& x)const
{
	
	return - 2.0f  * (x-m_center) / m_r  * src_value(x); 
}

double Cost3DMock::ref_value(const C3DFVector& x)const 
{
	const C3DFVector p = x - m_center - C3DFVector(1.1,1.2,1.1); 
	return exp( - p.norm2() / m_r ); 
}


BOOST_AUTO_TEST_CASE (test_iterator_range) 
{
	
}
