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



#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/gsl_pca.hh>

using namespace gsl; 

BOOST_AUTO_TEST_CASE( test_matrix_pca ) 
{
        const double input[50]  = { 
                1, 2, 3, 2, 1, 
		2, 1, 2, 5, 2, 
		3, 2, 5, 2, 5, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 

	Matrix m(10, 5, input); 

        PCA pca(3, 0.0);
        
        auto result = pca.analyze(m.transposed());
        BOOST_CHECK_EQUAL(result.eval.size(), 3u); 
        BOOST_CHECK_CLOSE(result.eval[0], 8.785043, 0.1); 
        BOOST_CHECK_CLOSE(result.eval[1], 5.462723, 0.1); 
        BOOST_CHECK_CLOSE(result.eval[2], 2.673904, 0.1); 
        

        BOOST_CHECK_EQUAL(result.evec.rows(), 5u); 
        BOOST_CHECK_EQUAL(result.evec.cols(), 3u); 
        

        // note that the sign is undefined, the test should honor this 
        const double test_ev[15] = {
                0.01448099,  -0.03862017, -0.07748999,
                0.15538277,   0.55340859,  0.48156179,
                -0.42010386,  0.47788917, -0.73357794,
                0.55943565,  -0.44087868, -0.42617084,
                0.69727398,   0.51912941, -0.20575562
        }; 

        const double *t = test_ev; 
        for (unsigned r = 0; r < 5; ++r)
                for (unsigned c = 0; c < 3; ++c, ++t) {
                        BOOST_CHECK_CLOSE(result.evec(r,c), *t, 0.1); 
                }
                

        
}
