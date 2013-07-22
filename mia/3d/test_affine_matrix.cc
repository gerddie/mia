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
	BOOST_CHECK_SMALL(inv_data[1], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[2], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[3], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[4], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[5], 2.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[6], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[7], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[8], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[9], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[10], 5.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[11], 1e-5f); 

	BOOST_CHECK_SMALL(inv_data[12], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[13], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[14], 1e-5f); 
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
	BOOST_CHECK_CLOSE(inv_data[1], 1.0f/42.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[2], 5.0f/14.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[3], -43.0f/84.0f, 0.01);

	BOOST_CHECK_CLOSE(inv_data[4], 1.0f/2.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[5], -1.0f/3.0f, 0.01);
	BOOST_CHECK_SMALL(inv_data[6], 1e-5f);
	BOOST_CHECK_CLOSE(inv_data[7], 7.0f/6.0f, 0.01);


	BOOST_CHECK_SMALL(inv_data[8], 1e-5f);

	BOOST_CHECK_CLOSE(inv_data[9], 10.0f/21.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[10], -5.0f/14.0f, 0.01);
	BOOST_CHECK_CLOSE(inv_data[11], -73.0f/42.0f, 0.01);

	BOOST_CHECK_SMALL(inv_data[12], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[13], 1e-5f); 
	BOOST_CHECK_SMALL(inv_data[14], 1e-5f); 
	BOOST_CHECK_CLOSE(inv_data[15], 1.0f, 0.01);
        
}

BOOST_AUTO_TEST_CASE( test_rot_x_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_x(C3DFVector(20.0f, 12.0f, 3.0f), M_PI/3.0f); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();


	BOOST_CHECK_CLOSE(data[0], 1.0, 0.01);
	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_SMALL(data[3], 1e-5f);

	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], cos_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[6], -sin_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[7], -12.0f * 0.5f - sin_pi_3 * 3.0f, 0.01f);

	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_CLOSE(data[9], sin_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[10],cos_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[11],12.0f * sin_pi_3 - 1.5f  , 0.01);

	BOOST_CHECK_SMALL(data[12], 1e-5f); 
	BOOST_CHECK_SMALL(data[13], 1e-5f); 
	BOOST_CHECK_SMALL(data[14], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
	

}

BOOST_AUTO_TEST_CASE( test_rot_y_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_y(C3DFVector(20.0f, 12.0f, 3.0f), M_PI/3.0f); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], cos_pi_3, 0.01f);
	BOOST_CHECK_SMALL(data[1], 1e-5f);
	BOOST_CHECK_CLOSE(data[2], -sin_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[3], -12.0f * 0.5f - sin_pi_3 * 3.0f, 0.01f);

	BOOST_CHECK_SMALL(data[4], 1e-5f);
	BOOST_CHECK_CLOSE(data[5], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_SMALL(data[7], 1e-5f);


	BOOST_CHECK_CLOSE(data[8], sin_pi_3, 0.01);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_CLOSE(data[10],cos_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[11],12.0f * sin_pi_3 - 1.5f  , 0.01);

	BOOST_CHECK_SMALL(data[12], 1e-5f); 
	BOOST_CHECK_SMALL(data[13], 1e-5f); 
	BOOST_CHECK_SMALL(data[14], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
}

BOOST_AUTO_TEST_CASE( test_rot_z_from_identity ) 
{
	
	CAffinTransformMatrix m; 
	
	m.rotate_z(C3DFVector(20.0f, 12.0f, 3.0f), M_PI/3.0f); 
	
	const float sin_pi_3 = sqrt(3.0f)/ 2.0f; 
	const float cos_pi_3 = 1.0f/ 2.0f; 
	
	const auto& data = m.data();

	BOOST_CHECK_CLOSE(data[0], cos_pi_3, 0.01f);
	BOOST_CHECK_CLOSE(data[1], -sin_pi_3, 0.01f);
	BOOST_CHECK_SMALL(data[2], 1e-5f);
	BOOST_CHECK_CLOSE(data[3], -12.0f * 0.5f - sin_pi_3 * 3.0f, 0.01f);


	BOOST_CHECK_CLOSE(data[4], sin_pi_3, 0.01);
	BOOST_CHECK_CLOSE(data[5],cos_pi_3, 0.01);
	BOOST_CHECK_SMALL(data[6], 1e-5f);
	BOOST_CHECK_CLOSE(data[7],12.0f * sin_pi_3 - 1.5f  , 0.01);


	BOOST_CHECK_SMALL(data[8], 1e-5f);
	BOOST_CHECK_SMALL(data[9], 1e-5f);
	BOOST_CHECK_CLOSE(data[10], 1.0f, 0.01);
	BOOST_CHECK_SMALL(data[11], 1e-5f);


	BOOST_CHECK_SMALL(data[12], 1e-5f); 
	BOOST_CHECK_SMALL(data[13], 1e-5f); 
	BOOST_CHECK_SMALL(data[14], 1e-5f); 
	BOOST_CHECK_CLOSE(data[15], 1.0f, 0.01);
	

}


