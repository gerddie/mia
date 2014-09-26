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


#include <gsl++/matrix_vector_ops.hh> 

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

using namespace gsl; 


BOOST_AUTO_TEST_CASE( test_vec_mult_marix ) 
{
        const double matrix_init[] = {
                1, 2, 3, 
                4, 5, 6
        };

        const double vector_init[] = {
                2, 3
        }; 

        Matrix rhs(2,3, matrix_init); 
        DoubleVector lhs(2, false); 
        DoubleVector result(3, true); 
        
        std::copy(vector_init, vector_init + 2, lhs.begin()); 
        
        multiply_v_m(result, lhs, rhs);

        BOOST_CHECK_CLOSE(result[0], 14, 0.1); 
        BOOST_CHECK_CLOSE(result[1], 19, 0.1); 
        BOOST_CHECK_CLOSE(result[2], 24, 0.1); 

}

BOOST_AUTO_TEST_CASE( test_mult_matrix_marix ) 
{

        const double lhs_init[] = {
                2, 3, 
                1, 2, 
                -1, -2, 
                4, 5
        }; 

        const double rhs_init[] = {
                1, 2, 3, 
                4, 5, 6
        };


        Matrix lhs(4, 2, lhs_init); 
        Matrix rhs(2, 3, rhs_init); 

        
        Matrix result(4, 3, true); 
        
        multiply_m_m(result, lhs, rhs);

        BOOST_CHECK_CLOSE(result(0, 0), 14, 0.1); 
        BOOST_CHECK_CLOSE(result(0, 1), 19, 0.1); 
        BOOST_CHECK_CLOSE(result(0, 2), 24, 0.1); 

        BOOST_CHECK_CLOSE(result(1, 0),  9, 0.1); 
        BOOST_CHECK_CLOSE(result(1, 1), 12, 0.1); 
        BOOST_CHECK_CLOSE(result(1, 2), 15, 0.1); 

        BOOST_CHECK_CLOSE(result(2, 0), -9, 0.1); 
        BOOST_CHECK_CLOSE(result(2, 1),-12, 0.1); 
        BOOST_CHECK_CLOSE(result(2, 2),-15, 0.1); 

        BOOST_CHECK_CLOSE(result(3, 0), 24, 0.1); 
        BOOST_CHECK_CLOSE(result(3, 1), 33, 0.1); 
        BOOST_CHECK_CLOSE(result(3, 2), 42, 0.1); 

}


BOOST_AUTO_TEST_CASE( test_mult_vec_marix ) 
{

        const double matrix_init[] = {
                1, 2, 
                3, 4, 
                5, 6
        };

        const double vector_init[] = {
                2, 3
        }; 


        Matrix lhs(3,2, matrix_init); 
        DoubleVector rhs(2, false); 
        DoubleVector result(3, true); 
        
        std::copy(vector_init, vector_init + 2, rhs.begin()); 
        
        multiply_m_v(result, lhs, rhs);

        BOOST_CHECK_CLOSE(result[0],  8, 0.1); 
        BOOST_CHECK_CLOSE(result[1], 18, 0.1); 
        BOOST_CHECK_CLOSE(result[2], 28, 0.1); 

}
