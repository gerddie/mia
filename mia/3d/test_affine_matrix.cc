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

#include <mia/internal/autotest.hh>
#include <mia/3d/affine_matrix.hh>

using namespace mia; 

BOOST_AUTO_TEST_CASE( test_identity ) 
{
        CAffinTransformMatrix identity; 
        
        C3DFVector x(1.0f, 2.0f, 3.0f); 
        const C3DFVector y = identity * x; 

        BOOST_CHECK_EQUAL(y, x);
        
}


BOOST_AUTO_TEST_CASE( test_simple_inverse ) 
{
	
	CAffinTransformMatrix simple(2.0f, 0.0f, 0.0f, 0.0f, 
				     0.0f, 0.5f, 0.0f, 0.0f, 
				     0.0f, 0.0f, 0.2f, 0.0f); 
        
	
	auto inv = simple.inverse(); 
	
	const auto& inv_data = inv.data(); 

	BOOST_CHECK_CLOSE(inv_data[0], 0.5f, 0.01);
	BOOST_CHECK_SMALL(inv_data[4], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[8], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[12], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[1], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[5], 2.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[9], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[13], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[2], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[6], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[10], 5.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[14], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[3], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[7], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[15], 1.0f, 0.01);
        
}

BOOST_AUTO_TEST_CASE( test_inverse ) 
{
	
	CAffinTransformMatrix simple(2.0f, 3.0f, 2.0f, 1.0f, 
				     3.0f, 1.5f, 3.0f, 5.0f, 
				     4.0f, 2.0f, 1.2f, 1.8f); 

	
	auto inv = simple.inverse(); 
	
	const auto& inv_data = inv.data(); 

	BOOST_CHECK_CLOSE(inv_data[0], -0.25f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[4], 1.0f/42.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[8], 5.0f/14.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[12], -43.0f/84.0f, 0.01);

	BOOST_CHECK_CLOSE(inv_data[1], 1.0f/2.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[5], -1.0f/3.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[9], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[13], 7.0f/6.0f, 0.01);


	BOOST_CHECK_SMALL(inv_data[2], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[6], 10.0f/21.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[10], -5.0f/14.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[14], -73.0f/42.0f, 0.01);

	BOOST_CHECK_SMALL(inv_data[3], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[7], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[15], 1.0f, 0.01);
        
}

BOOST_AUTO_TEST_CASE( test_rot_x_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_x(M_PI/3.0f, C3DFVector(20.0f, 12.0f, 3.0f)); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();


	BOOST_CHECK_CLOSE(data[0], 1.0, 0.01);
	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_SMALL(data[12], 1e-5f);

	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], cos_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[9], -sin_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[13], +12.0f * 0.5f + sin_pi_3 * 3.0f, 0.01f);

	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_CLOSE(data[6], sin_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[10],cos_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[14], -12.0f * sin_pi_3 + 1.5f  , 0.01);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
	

}

BOOST_AUTO_TEST_CASE( test_rot_y_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_y(M_PI/3.0f, C3DFVector(20.0f, 12.0f, 3.0f)); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], cos_pi_3, 0.01f);
	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_CLOSE(data[8], -sin_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[12], 20.0f * 0.5f + sin_pi_3 * 3.0f, 0.01f);

	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_SMALL(data[13], 1e-5f);


	BOOST_CHECK_CLOSE(data[2], sin_pi_3, 0.01);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10],cos_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[14],-20.0f * sin_pi_3 + 1.5f  , 0.01);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
}

BOOST_AUTO_TEST_CASE( test_rot_z_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_z(M_PI/3.0f, C3DFVector(20.0f, 12.0f, 3.0f)); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], cos_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[4], -sin_pi_3, 0.01f);
	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_CLOSE(data[12], 20.0f * 0.5f + sin_pi_3 * 12.0f, 0.01f);


	BOOST_CHECK_CLOSE(data[1], sin_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[5],cos_pi_3, 0.01);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_CLOSE(data[13], -20.0f * sin_pi_3 + 6.0f  , 0.01);


	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[14], 1e-5f);


	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
}

BOOST_AUTO_TEST_CASE( test_translate_from_identity ) 
{
	CAffinTransformMatrix m; 
	m.translate(C3DFVector(2.0f, 3.0f, 4.0f)); 
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], 1.0f, 0.01f);
	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_CLOSE(data[12], 2.0f, 0.01f);


	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01f);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_CLOSE(data[13], 3.0f, 0.01f);


	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01f);
	BOOST_CHECK_CLOSE(data[14], 4.0f, 0.01);


	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
	

	const auto inv = m.inverse(); 
	const auto& inv_data = inv.data();


	BOOST_CHECK_CLOSE(inv_data[0], 1.0f, 0.01f);
	BOOST_CHECK_SMALL(inv_data[4], 1e-5f);
	BOOST_CHECK_SMALL(inv_data[8], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[12], -2.0f, 0.01f);


	BOOST_CHECK_SMALL(inv_data[1], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[5], 1.0f, 0.01f);
	BOOST_CHECK_SMALL(inv_data[9], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[13], -3.0f, 0.01f);


	BOOST_CHECK_SMALL(inv_data[2], 1e-5f);
	BOOST_CHECK_SMALL(inv_data[6], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[10], 1.0f, 0.01f);
	BOOST_CHECK_CLOSE(inv_data[14], -4.0f, 0.01);


	BOOST_CHECK_SMALL(inv_data[3], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[7], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[15], 1.0f, 0.01);
}


struct ConcatationFixture {
	
	ConcatationFixture(); 
	void run_check(const CAffinTransformMatrix& m); 

	CAffinTransformMatrix start; 
	C3DFVector x; 
	C3DFVector y1;
}; 

ConcatationFixture::ConcatationFixture():
	start(2.0f, 1.0f, 3.0f, 1.0f, 
	      2.0f, 0.5f, 4.0f, 2.0f, 
	      3.0f, 4.0f, 0.2f, 1.0f), 
	x(2.0, 4.0, 9.0), 
	y1(start * x)
	
{
}

void ConcatationFixture::run_check(const CAffinTransformMatrix& m)
{
	C3DFVector y2_direct = start * x;
	C3DFVector y2_step = m * y1; 

	BOOST_CHECK_CLOSE(y2_direct.x, y2_step.x, 0.01); 
	BOOST_CHECK_CLOSE(y2_direct.y, y2_step.y, 0.01); 
	BOOST_CHECK_CLOSE(y2_direct.z, y2_step.z, 0.01); 
	
}

BOOST_FIXTURE_TEST_CASE( test_translate , ConcatationFixture) 
{
	C3DFVector t(1.0, 2.0, 3.0); 
	start.translate(t); 

	CAffinTransformMatrix m; 
	m.translate(t); 
	
	run_check(m); 
}

BOOST_AUTO_TEST_CASE( test_scale_from_identity ) 
{
	CAffinTransformMatrix m;
	
	C3DFVector center(1,2,3); 
	m.scale(C3DFVector(2.0f, 3.0f, 4.0f), center); 

	
	const auto& data = m.data(); 

	BOOST_CHECK_CLOSE(data[0], 2.0f, 0.01f);
	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_CLOSE(data[12], -1.0f, 0.01f);

	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 3.0f, 0.01f);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_CLOSE(data[13], -4.0f, 0.01f);

	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 4.0f, 0.01f);
	BOOST_CHECK_CLOSE(data[14],-9.0f, 0.01);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
	
}


BOOST_AUTO_TEST_CASE( test_rot_from_quaternion_and_identity ) 
{
	CAffinTransformMatrix m; 

	C3DFVector center(20.0f, 12.0f, 3.0f); 
	Quaternion q(sqrt(10.0f)/4.0f, 0.25f, -0.25f, 0.5f); 
	
	m.rotate(q, center);
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], 0.375f, 0.01f);
	BOOST_CHECK_CLOSE(data[4], 2 * ( -1/16.0- sqrt(10.0)/8.0)  , 0.01f);
	BOOST_CHECK_CLOSE(data[8], 2 * (0.125 - sqrt(10.0)/16.0 ), 0.01f );
	BOOST_CHECK_CLOSE(data[12], 20.0f - (data[0] * 20.0f + data[4] *12.0f + data[8] * 3.0f  ), 0.01f);


	BOOST_CHECK_CLOSE(data[1], 2 * ( sqrt(10.0)/8.0 -1/16.0) , 0.01);
	BOOST_CHECK_CLOSE(data[5], 0.375f, 0.01);
	BOOST_CHECK_CLOSE(data[9], 2 * (-.125 - sqrt(10.0)/16.0), 0.01f);
	BOOST_CHECK_CLOSE(data[13], 12.0f - (data[1] * 20 + data[5] *12.0f + data[9] * 3.0f  ) , 0.01);


	BOOST_CHECK_CLOSE(data[2], 2 * (1/8.0 + sqrt(10.0)/16.0), 0.01f);
	BOOST_CHECK_CLOSE(data[6], 2 * (-1/8.0 + sqrt(10.0)/16.0), 0.01f);
	BOOST_CHECK_CLOSE(data[10], 3.0f/4.0f, 0.01);
	BOOST_CHECK_CLOSE(data[14], 3.0f - (data[2] * 20 + data[6] *12.0f + data[10]* 3.0f  ), 0.01f);


	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
}


BOOST_FIXTURE_TEST_CASE( test_rot_from_quaternion_and_pseudorando_matrix , ConcatationFixture) 
{
	
	C3DFVector center(20.0f, 12.0f, 3.0f); 
	Quaternion q(sqrt(10.0f)/4.0f, 0.25f, -0.25f, 0.5f); 

	CAffinTransformMatrix m; 	
	m.rotate(q, center);
	start.rotate(q, center);
	
	run_check(m); 
}


BOOST_FIXTURE_TEST_CASE( test_rot_x_pseudorando_matrix , ConcatationFixture) 
{
	
	C3DFVector center(20.0f, 12.0f, 3.0f); 

	CAffinTransformMatrix m; 	
	m.rotate_x(1.0, center);
	start.rotate_x(1.0, center);
	
	run_check(m); 
}

BOOST_FIXTURE_TEST_CASE( test_rot_y_pseudorando_matrix , ConcatationFixture) 
{
	
	C3DFVector center(20.0f, 12.0f, 3.0f); 

	CAffinTransformMatrix m; 	
	m.rotate_y(1.0, center);
	start.rotate_y(1.0, center);
	
	run_check(m); 
}

BOOST_FIXTURE_TEST_CASE( test_rot_z_pseudorando_matrix , ConcatationFixture) 
{
	
	C3DFVector center(20.0f, 12.0f, 3.0f); 

	CAffinTransformMatrix m; 	
	m.rotate_z(1.0, center);
	start.rotate_z(1.0, center);
	
	run_check(m); 
}

BOOST_FIXTURE_TEST_CASE( test_scale_pseudorando_matrix , ConcatationFixture) 
{
	
	C3DFVector center(20.0f, 12.0f, 3.0f); 
	C3DFVector scales(2.0f, 12.0f, 0.5f); 

	CAffinTransformMatrix m; 	
	m.scale(scales, center);
	start.scale(scales, center);
	
	run_check(m); 
}


BOOST_AUTO_TEST_CASE( test_multiply ) 
{
	CAffinTransformMatrix lhs(2.0f, 1.0f, 3.0f, 1.0f, 
				  2.0f, 0.5f, 4.0f, 2.0f, 
				  3.0f, 4.0f, 0.2f, 1.0f); 

	CAffinTransformMatrix rhs(3.0f, 4.0f, 1.0f, 7.0f, 
				  2.0f, 1.5f, 2.0f, 3.0f, 
				  3.5f, 2.0f, 2.2f, 2.0f);

	lhs *= rhs; 
	
	const auto& data = lhs.data();

	BOOST_CHECK_CLOSE(data[0], 18.5, 0.01);
	BOOST_CHECK_CLOSE(data[4], 15.5, 0.01);
	BOOST_CHECK_CLOSE(data[8], 10.6, 0.01);
	BOOST_CHECK_CLOSE(data[12], 24.0, 0.01);

	BOOST_CHECK_CLOSE(data[1], 21.0, 0.01);
	BOOST_CHECK_CLOSE(data[5], 16.75, 0.01);
	BOOST_CHECK_CLOSE(data[9], 11.8, 0.01);
	BOOST_CHECK_CLOSE(data[13], 25.5, 0.01);

	BOOST_CHECK_CLOSE(data[2], 17.7, 0.01);
	BOOST_CHECK_CLOSE(data[6], 18.4, 0.01);
	BOOST_CHECK_CLOSE(data[10], 11.44, 0.01);
	BOOST_CHECK_CLOSE(data[14], 34.4, 0.01);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_EQUAL(data[15], 1.0f);
	
}

#if 0 
// not yet supported, will be added if needed 
BOOST_AUTO_TEST_CASE( test_shear ) 
{
	CAffinTransformMatrix m; 
	m.shear(1.1, 1.5, 0.5); 

	C3DFVector x(1,2,3); 

	auto y = m * x; 
	
	BOOST_CHECK_CLOSE(y.x, 5.6f, 0.01f);
	BOOST_CHECK_CLOSE(y.y, 4.5f, 0.01f);
	BOOST_CHECK_CLOSE(y.z, 3.0f, 0.01f);


}
#endif 

BOOST_AUTO_TEST_CASE( test_multiply_const_input ) 
{
	const CAffinTransformMatrix lhs(2.0f, 1.0f, 3.0f, 1.0f, 
				  2.0f, 0.5f, 4.0f, 2.0f, 
				  3.0f, 4.0f, 0.2f, 1.0f); 

	const CAffinTransformMatrix rhs(3.0f, 4.0f, 1.0f, 7.0f, 
				  2.0f, 1.5f, 2.0f, 3.0f, 
				  3.5f, 2.0f, 2.2f, 2.0f);

	auto r = lhs * rhs; 
	
	const auto& data = r.data();

	BOOST_CHECK_CLOSE(data[0], 18.5, 0.01);
	BOOST_CHECK_CLOSE(data[4], 15.5, 0.01);
	BOOST_CHECK_CLOSE(data[8], 10.6, 0.01);
	BOOST_CHECK_CLOSE(data[12], 24.0, 0.01);

	BOOST_CHECK_CLOSE(data[1], 21.0, 0.01);
	BOOST_CHECK_CLOSE(data[5], 16.75, 0.01);
	BOOST_CHECK_CLOSE(data[9], 11.8, 0.01);
	BOOST_CHECK_CLOSE(data[13], 25.5, 0.01);

	BOOST_CHECK_CLOSE(data[2], 17.7, 0.01);
	BOOST_CHECK_CLOSE(data[6], 18.4, 0.01);
	BOOST_CHECK_CLOSE(data[10], 11.44, 0.01);
	BOOST_CHECK_CLOSE(data[14], 34.4, 0.01);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_EQUAL(data[15], 1.0f);
	
}

BOOST_AUTO_TEST_CASE( test_shear_at_zero ) 
{
	CAffinTransformMatrix m; 
	m.shear(C3DFVector(2,3,4)); 

	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], 1.0f, 0.01);
	BOOST_CHECK_CLOSE(data[4], 2.0f, 0.01);
	BOOST_CHECK_SMALL(data[8], 1e-5f); 
	BOOST_CHECK_SMALL(data[12], 1e-5f); 


	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01);
	BOOST_CHECK_CLOSE(data[9], 3.0f, 0.01);
	BOOST_CHECK_SMALL(data[13], 1e-5f);

	BOOST_CHECK_CLOSE(data[2], 4.0f, 0.01);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[14], 1e-5f);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_EQUAL(data[15], 1.0f);


}

BOOST_AUTO_TEST_CASE( test_zero_shear_centered ) 
{
	CAffinTransformMatrix m; 
	m.shear(C3DFVector::_0, C3DFVector(2,3,4));

	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_SMALL(data[8], 1e-5f); 
	BOOST_CHECK_SMALL(data[12], 1e-5f); 


	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_SMALL(data[13], 1e-5f);

	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[14], 1e-5f);

	BOOST_CHECK_SMALL(data[3], 1e-5f); 
	BOOST_CHECK_SMALL(data[7], 1e-5f); 
	BOOST_CHECK_SMALL(data[11], 1e-5f); 
	BOOST_CHECK_EQUAL(data[15], 1.0f);
}


BOOST_AUTO_TEST_CASE( test_shear_centered ) 
{
	CAffinTransformMatrix m; 
	C3DFVector center(2,3,4);

	m.shear(C3DFVector(0.2,0.3,0.4), center);

	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], 1.0f, 0.01);
	BOOST_CHECK_CLOSE(data[4], 0.2f, 1e-5f);
	BOOST_CHECK_SMALL(data[8], 1e-5f); 
	BOOST_CHECK_SMALL(data[12], 1e-5f); 


	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01);
	BOOST_CHECK_CLOSE(data[9], 0.3f, 1e-5f);
	BOOST_CHECK_SMALL(data[13], 1e-5f);

	BOOST_CHECK_CLOSE(data[2], 0.4f, 1e-5f);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[14], 1e-5f);

	BOOST_CHECK_CLOSE(data[3], -0.6, 1e-4f); 
	BOOST_CHECK_CLOSE(data[7], -1.2, 1e-4f); 
	BOOST_CHECK_CLOSE(data[11],-0.8, 1e-4f); 
	BOOST_CHECK_EQUAL(data[15], 1.0f);
}

BOOST_AUTO_TEST_CASE( test_transform_centered ) 
{
	CAffinTransformMatrix lhs(2.0f, 1.0f, 3.0f, 1.0f, 
				  2.0f, 8.0f, 4.0f, 2.0f, 
				  3.0f, 4.0f, 6.0f, 1.0f); 
	
	C3DFMatrix m(C3DFVector(2.0, 3.0, 5.0), 
		     C3DFVector(4.0, 1.0, 8.0), 
		     C3DFVector(6.0, 7.0, 9.0)); 
	

	C3DFVector c(10,11,12);
	
	lhs.transform_centered(m, c);

	const auto& data = lhs.data();

	BOOST_CHECK_CLOSE(data[0], 25, 0.01);
	BOOST_CHECK_CLOSE(data[1], 34, 0.01);
	BOOST_CHECK_CLOSE(data[2], 53, 0.01);
	BOOST_CHECK_SMALL(data[3], 1e-5f); 

	BOOST_CHECK_CLOSE(data[4], 46, 0.01);
	BOOST_CHECK_CLOSE(data[5], 44, 0.01);
	BOOST_CHECK_CLOSE(data[6], 98, 0.01);
	BOOST_CHECK_SMALL(data[7], 1e-5f); 

	BOOST_CHECK_CLOSE(data[8], 48, 0.01);
	BOOST_CHECK_CLOSE(data[9], 64, 0.01);
	BOOST_CHECK_CLOSE(data[10], 100, 0.01);
	BOOST_CHECK_SMALL(data[11], 1e-5f); 

	BOOST_CHECK_CLOSE(data[12], -90, 0.01);
	BOOST_CHECK_CLOSE(data[13], -122, 0.01);
	BOOST_CHECK_CLOSE(data[14], -204, 0.01);
	BOOST_CHECK_EQUAL(data[15], 1.0f);
	
}
