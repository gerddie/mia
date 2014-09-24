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
#include <gsl++/matrix.hh>

using namespace gsl; 
using namespace ::boost;
using namespace ::boost::unit_test;



BOOST_AUTO_TEST_CASE( test_matrix_alloc_and_free ) 
{
	Matrix m(2,3, true); 
	BOOST_CHECK_EQUAL(m.rows(), 2u); 
	BOOST_CHECK_EQUAL(m.cols(), 3u); 

	for(size_t i = 0; i < 2;++i) 
		for(size_t j = 0; j < 2;++j) 
			BOOST_CHECK_EQUAL(m(i,j), 0.0); 

	m.set(1,2, 1.0); 
	BOOST_CHECK_EQUAL(m(1,2), 1.0); 
	
	Matrix k(m); 
	BOOST_CHECK_EQUAL(k.rows(), 2u); 
	BOOST_CHECK_EQUAL(k.cols(), 3u); 
	BOOST_CHECK_EQUAL(k(1,2), 1.0); 

	k.set(0,2, 2.0); 
	BOOST_CHECK_EQUAL(k(0,2), 2.0); 

	m = k; 
	BOOST_CHECK_EQUAL(k(0,2), 2.0); 

}


BOOST_AUTO_TEST_CASE( test_with_init ) 
{
	const double input[6]  = { 
		1,2,3,4,5,6
	}; 
	
	Matrix m(2,3, input); 

	BOOST_CHECK_EQUAL(m(0,0), 1); 
	BOOST_CHECK_EQUAL(m(0,1), 2); 
	BOOST_CHECK_EQUAL(m(0,2), 3); 
	BOOST_CHECK_EQUAL(m(1,0), 4); 
	BOOST_CHECK_EQUAL(m(1,1), 5); 
	BOOST_CHECK_EQUAL(m(1,2), 6); 
	
}

BOOST_AUTO_TEST_CASE( test_matrix_covariance ) 
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
	
	auto cov = m.covariance(); 
	

	BOOST_CHECK_EQUAL(cov.rows(), 5); 
	BOOST_CHECK_EQUAL(cov.cols(), 5); 

	
	const double test[25]  = {  
		1.82222,  -0.11111,  -0.06667,   0.13333,   0.02222,
		-0.11111,   2.72222,   0.00000,  -0.94444,   2.11111,
		-0.06667,   0.00000,   4.26667,  -2.31111,  -0.86667,
		0.13333,  -0.94444,  -2.31111,   4.45556,   2.28889,
		0.02222,   2.11111,  -0.86667,   2.28889,   5.95556
	}; 
	
	auto t = test; 
	for (unsigned  r = 0; r < cov.rows(); ++r) 
		for (unsigned  c = 0; c < cov.cols(); ++c, ++t) {
			if (*t == 0.0) 
				BOOST_CHECK_SMALL(cov(r,c), 1e-10); 
			else 
				BOOST_CHECK_CLOSE(cov(r,c), *t, 0.1); 
		}
}
