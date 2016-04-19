/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <climits>

#define VSTREAM_DOMAIN "test-critical-points"
#include <mia/internal/autotest.hh>


#include <mia/3d/critical_point.hh>

using namespace mia; 

BOOST_AUTO_TEST_CASE( test_critical_point_simple )
{
        C3DCriticalPoint cp_0;

        BOOST_CHECK_EQUAL(cp_0.get_point(), C3DFVector::_0);
        BOOST_CHECK_EQUAL(cp_0.get_a(), C3DFMatrix::_0);
        BOOST_CHECK_EQUAL(cp_0.get_gamma(), 0.0f); 


        C3DFVector x(1,2,3); 
        C3DCriticalPoint cp_1(x);
        BOOST_CHECK_EQUAL(cp_1.get_point(), x);

        cp_1.set_a(C3DFMatrix::diagonal(C3DFVector(3.0f,2.0f,0.5f)));
        cp_1.set_gamma(0.1f);
        
        BOOST_CHECK_EQUAL(cp_1.get_gamma(), 0.1f);

        // check center 
        BOOST_CHECK_EQUAL(cp_1.at(x), C3DFVector::_0); 

        C3DFVector val_2_1_1 = cp_1.at(C3DFVector(2,1,1)); // delta = 1, -1, -2
        // C3DFVector test_2_1_1(0.1 * 3 / 6, 0.1f/3.0f,  -2 * 0.5 / 6 ); 

        BOOST_CHECK_CLOSE(val_2_1_1.x, 0.05f, 0.1);
        BOOST_CHECK_CLOSE(val_2_1_1.y, -0.1f/3.0f, 0.1);
        BOOST_CHECK_CLOSE(val_2_1_1.z, -0.1f/6.0f, 0.1);

	C3DFVector val_2_1_1_alt = cp_1.at_alt(C3DFVector(2,0,1)); //delta = 1, -2, -2
        // 3.9 
        // C3DFVector test_2_1_1(3 / 3.9    -4/ 3.9  -1/ 3.9  ); 

        BOOST_CHECK_CLOSE(val_2_1_1_alt.x,  3.0f / 3.9f, 0.1);
        BOOST_CHECK_CLOSE(val_2_1_1_alt.y, -4.0f / 3.9f, 0.1);
        BOOST_CHECK_CLOSE(val_2_1_1_alt.z, -1.0f / 3.9f, 0.1);
        
}

BOOST_AUTO_TEST_CASE( test_critical_point_eigen_zero )
{
        C3DCriticalPointEigen cp(C3DFVector::_1, C3DFMatrix::_0);

        BOOST_CHECK_EQUAL(cp.get_type(), C3DCriticalPointEigen::ev_zero); 
}

BOOST_AUTO_TEST_CASE( test_critical_point_eigen_real )
{
        C3DCriticalPointEigen cp(C3DFVector::_1,
                                 C3DFMatrix::diagonal(C3DFVector(1,2,3)));

        BOOST_CHECK_EQUAL(cp.get_type(), C3DCriticalPointEigen::ev_real);

        BOOST_CHECK_EQUAL(cp.get_eval1(), 3.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval3(), 1.0f);
        BOOST_CHECK_EQUAL(cp.get_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_eval3(), 1.0f); 

        BOOST_CHECK_EQUAL(cp.get_location(), C3DFVector::_1); 

        BOOST_CHECK_EQUAL(cp.get_evect1(), C3DFVector(0,0,1));
        BOOST_CHECK_EQUAL(cp.get_evect2(), C3DFVector(0,1,0));
        BOOST_CHECK_EQUAL(cp.get_evect3(), C3DFVector(1,0,0)); 
        BOOST_CHECK_EQUAL(cp.get_real_evect2(), C3DFVector(0,1,0));
        BOOST_CHECK_EQUAL(cp.get_real_evect3(), C3DFVector(1,0,0)); 

}

BOOST_AUTO_TEST_CASE( test_critical_point_eigen_real_two )
{
        C3DCriticalPointEigen cp(C3DFVector::_1,
                                 C3DFMatrix::diagonal(C3DFVector(2,2,3)));

        BOOST_CHECK_EQUAL(cp.get_type(), C3DCriticalPointEigen::ev_real_two_equal);

        BOOST_CHECK_EQUAL(cp.get_eval1(), 3.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval3(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_eval3(), 2.0f);
        
        BOOST_CHECK_EQUAL(cp.get_evect1(), C3DFVector(0,0,1));
        BOOST_CHECK_EQUAL(cp.get_evect2(), C3DFVector(1,0,0));
        BOOST_CHECK_EQUAL(cp.get_evect3(), C3DFVector(0,1,0)); 
        BOOST_CHECK_EQUAL(cp.get_real_evect2(), C3DFVector(1,0,0));
        BOOST_CHECK_EQUAL(cp.get_real_evect3(), C3DFVector(0,1,0)); 
        
}

BOOST_AUTO_TEST_CASE( test_critical_point_eigen_real_three )
{
        C3DCriticalPointEigen cp(C3DFVector::_1,
                                 C3DFMatrix::diagonal(C3DFVector(2,2,2)));

        BOOST_CHECK_EQUAL(cp.get_type(), C3DCriticalPointEigen::ev_real_three_equal);

        BOOST_CHECK_EQUAL(cp.get_eval1(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_real_eval3(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_eval2(), 2.0f);
        BOOST_CHECK_EQUAL(cp.get_eval3(), 2.0f);
        
        BOOST_CHECK_EQUAL(cp.get_evect1(), C3DFVector(0,0,1));
        BOOST_CHECK_EQUAL(cp.get_evect2(), C3DFVector(1,0,0));
        BOOST_CHECK_EQUAL(cp.get_evect3(), C3DFVector(0,1,0)); 
        BOOST_CHECK_EQUAL(cp.get_real_evect2(), C3DFVector(1,0,0));
        BOOST_CHECK_EQUAL(cp.get_real_evect3(), C3DFVector(0,1,0)); 
        
}
