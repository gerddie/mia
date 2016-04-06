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


#include <mia/core/gsl_matrix_vector_ops.hh> 

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <iostream>

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
        Vector lhs(2, false); 
        Vector result(3, true); 
        
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


BOOST_AUTO_TEST_CASE( test_mult_matrix_matrixT ) 
{

        const double lhs_init[] = {
                2, 3, 
                1, 2, 
                -1, -2, 
                4, 5
        }; 

        const double rhs_init[] = {
                1, 4,
                2, 5, 
		3, 6
        };


        Matrix lhs(4, 2, lhs_init); 
        Matrix rhs(3, 2, rhs_init); 

        
        Matrix result(4, 3, true); 
        
        multiply_m_mT(result, lhs, rhs);

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

BOOST_AUTO_TEST_CASE( test_mult_matrixT_martix ) 
{

        const double lhs_init[] = {
		2, 1, -1, 4, 
		3, 2, -2, 5
        }; 

        const double rhs_init[] = {
                1, 2, 3, 
		4, 5, 6
        };


        Matrix lhs(2, 4, lhs_init); 
        Matrix rhs(2, 3, rhs_init); 

        
        Matrix result(4, 3, true); 
        
        multiply_mT_m(result, lhs, rhs);

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
        Vector rhs(2, false); 
        Vector result(3, true); 
        
        std::copy(vector_init, vector_init + 2, rhs.begin()); 
        
        multiply_m_v(result, lhs, rhs);

        BOOST_CHECK_CLOSE(result[0],  8, 0.1); 
        BOOST_CHECK_CLOSE(result[1], 18, 0.1); 
        BOOST_CHECK_CLOSE(result[2], 28, 0.1); 

}

BOOST_AUTO_TEST_CASE( test_mult_vec_vec ) 
{
        const double vector1_init[] = {
                1, 2, 
        };

        const double vector2_init[] = {
                2, 3
        }; 


        Vector lhs(2, false); 
        Vector rhs(2, false); 
        
        std::copy(vector1_init, vector1_init + 2, lhs.begin()); 
        std::copy(vector2_init, vector2_init + 2, rhs.begin()); 
        
        BOOST_CHECK_CLOSE(dot(lhs, rhs), 8, 0.1); 
}


BOOST_AUTO_TEST_CASE( test_matrix_orthogonalize ) 
{
        const double U_init[] = {
                1.0, 0, 0, 0, 
		0, 1.0/sqrt(2.0), 1.0/sqrt(2.0), 0, 
		0, 1.0/sqrt(2.0), -1.0/sqrt(2.0), 0, 
		0, 0, 0, 1.0 
        };
	
        const double V_init[] = {
                1.0/sqrt(2.0), 1.0/sqrt(2.0), 0, 
		0, 1.0/sqrt(2.0), -1.0/sqrt(2.0), 0, 
		0, 0,  1.0
        }; 
	
	const double D_init[] = {
		2, 0, 0, 
		0, 3, 0, 
		0, 0, 4, 
		0, 0, 0
	}; 
	
	Matrix U(4, 4, U_init);
	Matrix V(3, 3, V_init);
	Matrix D(4, 3, D_init);
	Matrix temp(4, 3, false);
		
	Matrix M(4,3, false);
		
	multiply_m_m(temp, D, V);
	multiply_m_m(M, U, temp);
	
	matrix_orthogonalize(M); 
	
	
	for (int c = 0; c < 3; ++c) {
		auto col_1 = gsl_matrix_column(M, c);
		// should be unity
		BOOST_CHECK_CLOSE(dot(&col_1.vector, &col_1.vector), 1.0, 0.1); 
		for (int c2 = 0; c2 < c; ++c2) {
			auto col_2 = gsl_matrix_column(M, c2); 
			BOOST_CHECK_SMALL(dot(&col_1.vector, &col_2.vector), 1e-10); 
			
		}

	}


}
