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

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <mia/core/gsl_matrix.hh>

#include <stdexcept>

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

BOOST_AUTO_TEST_CASE( test_copy_ops ) 
{
	Matrix m(2,3, 2.0); 

	BOOST_CHECK_EQUAL(m(0,0), 2.0); 
	BOOST_CHECK_EQUAL(m(0,1), 2.0); 
	BOOST_CHECK_EQUAL(m(0,2), 2.0); 
	BOOST_CHECK_EQUAL(m(1,0), 2.0); 
	BOOST_CHECK_EQUAL(m(1,1), 2.0); 
	BOOST_CHECK_EQUAL(m(1,2), 2.0);

	Matrix m2(2,3, 1.0);

	BOOST_CHECK_EQUAL(m2.rows(), 2u); 
	BOOST_CHECK_EQUAL(m2.cols(), 3u); 
			  
	m2.reset(3, 2, 3.0);

	BOOST_CHECK_EQUAL(m2.rows(), 3u); 
	BOOST_CHECK_EQUAL(m2.cols(), 2u);
	
	BOOST_CHECK_EQUAL(m2(0,0), 3.0); 
	BOOST_CHECK_EQUAL(m2(0,1), 3.0); 
	BOOST_CHECK_EQUAL(m2(1,0), 3.0); 
	BOOST_CHECK_EQUAL(m2(1,1), 3.0); 
	BOOST_CHECK_EQUAL(m2(2,0), 3.0); 
	BOOST_CHECK_EQUAL(m2(2,1), 3.0);

	Matrix *m3 = &m2;
	m3->reset(2, 3, 4.0); 
	
	m2 = *m3;

	BOOST_CHECK_EQUAL(m2.rows(), 2u); 
	BOOST_CHECK_EQUAL(m2.cols(), 3u);
	
	BOOST_CHECK_EQUAL(m2(0,0), 4.0); 
	BOOST_CHECK_EQUAL(m2(0,1), 4.0); 
	BOOST_CHECK_EQUAL(m2(0,2), 4.0); 
	BOOST_CHECK_EQUAL(m2(1,0), 4.0); 
	BOOST_CHECK_EQUAL(m2(1,1), 4.0); 
	BOOST_CHECK_EQUAL(m2(1,2), 4.0);

	m2 = m;

	BOOST_CHECK_EQUAL(m2.rows(), 2u); 
	BOOST_CHECK_EQUAL(m2.cols(), 3u); 
	
	BOOST_CHECK_EQUAL(m2(0,0), 2.0); 
	BOOST_CHECK_EQUAL(m2(0,1), 2.0); 
	BOOST_CHECK_EQUAL(m2(0,2), 2.0); 
	BOOST_CHECK_EQUAL(m2(1,0), 2.0); 
	BOOST_CHECK_EQUAL(m2(1,1), 2.0); 
	BOOST_CHECK_EQUAL(m2(1,2), 2.0);
	
	
}

BOOST_AUTO_TEST_CASE( test_const_iterator )
{
	const double input[6]  = { 
		1,2,3,4,5,6
	}; 
	
	Matrix m(2,3, input);

	int v = 1; 
	for ( auto i = m.begin(); i != m.end(); ++i, ++v) {
		BOOST_CHECK_EQUAL(*i, v); 
	}

	auto i = m.begin();
	BOOST_CHECK(i == m.begin()); 
}

BOOST_AUTO_TEST_CASE( test_sum )
{
	const double input1[6]  = { 
		1,2,3,4,5,6
	};
	
	const double input2[6]  = { 
		5,4,3,2,1,0
	}; 

	
	Matrix m1(2, 3, input1);
	Matrix m2(2, 3, input2);

	Matrix m3 = m1 + m2;

	BOOST_CHECK_EQUAL(m3.rows(), 2u); 
	BOOST_CHECK_EQUAL(m3.cols(), 3u); 
	
	BOOST_CHECK_EQUAL(m3(0,0), 6.0); 
	BOOST_CHECK_EQUAL(m3(0,1), 6.0); 
	BOOST_CHECK_EQUAL(m3(0,2), 6.0); 
	BOOST_CHECK_EQUAL(m3(1,0), 6.0); 
	BOOST_CHECK_EQUAL(m3(1,1), 6.0); 
	BOOST_CHECK_EQUAL(m3(1,2), 6.0);
	
}

BOOST_AUTO_TEST_CASE( test_sum_fail )
{
	const double input1[6]  = { 
		1,2,3,4,5,6
	};
	
	const double input2[6]  = { 
		5,4,3,2,1,0
	}; 

	
	Matrix m1(2, 3, input1);
	Matrix m2(3, 2, input2);

	// this must fail 
	BOOST_CHECK_THROW(m1 + m2, std::logic_error); 

	
}



BOOST_AUTO_TEST_CASE( test_transpose ) 
{
	const double input[6]  = { 
		1,2,3,4,5,6
	}; 
	
	Matrix m(2,3, input); 
	
	auto t = m.transposed(); 

	BOOST_CHECK_EQUAL(t.rows(), 3); 
	BOOST_CHECK_EQUAL(t.cols(), 2); 
	
	BOOST_CHECK_EQUAL(t(0,0), 1); 
	BOOST_CHECK_EQUAL(t(1,0), 2); 
	BOOST_CHECK_EQUAL(t(2,0), 3); 
	BOOST_CHECK_EQUAL(t(0,1), 4); 
	BOOST_CHECK_EQUAL(t(1,1), 5); 
	BOOST_CHECK_EQUAL(t(2,1), 6); 
	
}


BOOST_AUTO_TEST_CASE( test_matrix_column_covariance ) 
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
	
	auto cov = m.column_covariance(); 
	

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

BOOST_AUTO_TEST_CASE( test_matrix_row_covariance ) 
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
	
	auto cov = m.transposed().row_covariance(); 
	

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

BOOST_AUTO_TEST_CASE( test_matrix_iterator ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 

	const double test_submatrix[21]  = { 
	 	12, 13, 14,
		6, 2, 3, 
		1, 6, 2,
		1, 2, 5,
		2, 3, 8,
		4, 8, 1,
		2, 3, 2
	}; 

	Matrix m(10, 5, input); 

	auto im = m.begin(); 
	for (int i = 0; i < 50; ++i, ++im) {
		BOOST_CHECK_EQUAL(*im, input[i]); 
	}

	// look at a view 

	gsl_matrix_view mv = gsl_matrix_submatrix (m, 2, 1, 7, 3); 
	
	Matrix mvm(&mv.matrix); 
	
	auto imvm = mvm.begin(); 
	for (int i = 0; i < 21; ++i, ++imvm) {
		BOOST_CHECK_EQUAL(*imvm, test_submatrix[i]); 
	}
	
	for(auto im2 = m.begin(); im2 != m.end(); ++im2) 
		*im2 = 1; 

	int k = 0; 
	for (auto imvm2 = mvm.begin(); imvm2 != mvm.end(); ++imvm2, ++k) {
		BOOST_CHECK_EQUAL(*imvm, 1); 
	}
	BOOST_CHECK_EQUAL(k, 21); 

	

}

BOOST_AUTO_TEST_CASE( test_const_matrix_iterator ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 

	const double test_submatrix[21]  = { 
	 	12, 13, 14,
		6, 2, 3, 
		1, 6, 2,
		1, 2, 5,
		2, 3, 8,
		4, 8, 1,
		2, 3, 2
	}; 

	const Matrix m(10, 5, input); 

	auto im = m.begin(); 
	for (int i = 0; i < 50; ++i, ++im) {
		BOOST_CHECK_EQUAL(*im, input[i]); 
	}

	// look at a view 

	gsl_matrix_const_view mv = gsl_matrix_const_submatrix (m, 2, 1, 7, 3); 
	
	const Matrix mvm(&mv.matrix); 
	
	auto imvm = mvm.begin(); 
	for (int i = 0; i < 21; ++i, ++imvm) {
		BOOST_CHECK_EQUAL(*imvm, test_submatrix[i]); 
	}
	

}

BOOST_AUTO_TEST_CASE( test_row_ops ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 
	
	Matrix m(10, 5, input); 

	auto mr = m.get_row(2); 
	BOOST_CHECK_EQUAL(mr.size(), 5); 

	BOOST_CHECK_EQUAL(mr[0], 11); 
	BOOST_CHECK_EQUAL(mr[1], 12); 
	BOOST_CHECK_EQUAL(mr[2], 13); 
	BOOST_CHECK_EQUAL(mr[3], 14); 
	BOOST_CHECK_EQUAL(mr[4], 15); 
	
	auto b = mr.begin(); 

	BOOST_CHECK_EQUAL(*b, 11); ++b; 
	BOOST_CHECK_EQUAL(*b, 12); ++b; 
	BOOST_CHECK_EQUAL(*b, 13); ++b; 
	BOOST_CHECK_EQUAL(*b, 14); ++b; 
	BOOST_CHECK_EQUAL(*b, 15); ++b; 
	
	BOOST_CHECK(b == mr.end()); 

	m.set_row(3, mr); 
	auto mr3 = m.get_row(3); 
	BOOST_CHECK_EQUAL(mr3.size(), 5); 

	BOOST_CHECK_EQUAL(mr3[0], 11); 
	BOOST_CHECK_EQUAL(mr3[1], 12); 
	BOOST_CHECK_EQUAL(mr3[2], 13); 
	BOOST_CHECK_EQUAL(mr3[3], 14); 
	BOOST_CHECK_EQUAL(mr3[4], 15); 
	

}

BOOST_AUTO_TEST_CASE( test_const_row_ops ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 
	
	const Matrix m(10, 5, input); 

	auto mr = m.get_row(2); 
	BOOST_CHECK_EQUAL(mr.size(), 5); 

	BOOST_CHECK_EQUAL(mr[0], 11); 
	BOOST_CHECK_EQUAL(mr[1], 12); 
	BOOST_CHECK_EQUAL(mr[2], 13); 
	BOOST_CHECK_EQUAL(mr[3], 14); 
	BOOST_CHECK_EQUAL(mr[4], 15); 
	
	auto b = mr.begin(); 

	BOOST_CHECK_EQUAL(*b, 11); ++b; 
	BOOST_CHECK_EQUAL(*b, 12); ++b; 
	BOOST_CHECK_EQUAL(*b, 13); ++b; 
	BOOST_CHECK_EQUAL(*b, 14); ++b; 
	BOOST_CHECK_EQUAL(*b, 15); ++b; 
	
	BOOST_CHECK(b == mr.end()); 

	double v_init[5] = {1, 2, 3, 4, 5}; 
	
	Vector v(5, v_init);

	BOOST_CHECK_EQUAL(m.dot_row(9, v), 46.0); 
}


BOOST_AUTO_TEST_CASE( test_const_col_ops ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 
	
	const Matrix m(10, 5, input); 

	auto mr = m.get_column(2); 
	BOOST_CHECK_EQUAL(mr.size(), 10); 

	BOOST_CHECK_EQUAL(mr[0], 3); 
	BOOST_CHECK_EQUAL(mr[1], 8); 
	BOOST_CHECK_EQUAL(mr[2], 13); 
	BOOST_CHECK_EQUAL(mr[3], 2); 
	BOOST_CHECK_EQUAL(mr[4], 6); 

	BOOST_CHECK_EQUAL(mr[5], 2); 
	BOOST_CHECK_EQUAL(mr[6], 3); 
	BOOST_CHECK_EQUAL(mr[7], 8); 
	BOOST_CHECK_EQUAL(mr[8], 3); 
	BOOST_CHECK_EQUAL(mr[9], 2); 	

	
	auto b = mr.begin(); 

	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 8); ++b; 
	BOOST_CHECK_EQUAL(*b, 13); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 
	BOOST_CHECK_EQUAL(*b, 6); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 
	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 8); ++b; 
	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 

	BOOST_CHECK(b == mr.end()); 

}


BOOST_AUTO_TEST_CASE( test_matrix_inv_sqrt ) 
{
	const double input[16]  = {
		0.25, 0.35, 0.1, 0.3, 
		0.35, 0.25, 0.1, 0.3, 
		0.1,  0.3, 0.25, 0.35,  
		0.15, 0.45, 0.2, 0.7
	}; 

	Matrix m(4,4,input); 
	matrix_inv_sqrt(m); 

	       
	BOOST_CHECK_CLOSE(m(0,0), 0.272403, 0.1); 
	BOOST_CHECK_CLOSE(m(0,1), 0.919640, 0.1);
	BOOST_CHECK_CLOSE(m(0,2),-0.228760, 0.1);
	BOOST_CHECK_CLOSE(m(0,3),-0.166518, 0.1);

	BOOST_CHECK_CLOSE(m(1,0), 0.937965, 0.1);
	BOOST_CHECK_CLOSE(m(1,1),-0.201772, 0.1);
	BOOST_CHECK_CLOSE(m(1,2), 0.120041, 0.1);
	BOOST_CHECK_CLOSE(m(1,3), 0.255148, 0.1);

	BOOST_CHECK_CLOSE(m(2,0), -0.079173, 0.1);
	BOOST_CHECK_CLOSE(m(2,1), 0.271666, 0.1);
	BOOST_CHECK_CLOSE(m(2,2), 0.957528, 0.1);
	BOOST_CHECK_CLOSE(m(2,3), 0.055393, 0.1);

	BOOST_CHECK_CLOSE(m(3,0), -0.199374, 0.1);
	BOOST_CHECK_CLOSE(m(3,1), 0.199370, 0.1);
	BOOST_CHECK_CLOSE(m(3,2), -0.128056, 0.1);
	BOOST_CHECK_CLOSE(m(3,3), 0.950843, 0.1);

	
}

BOOST_AUTO_TEST_CASE( test_col_ops ) 
{
	const double input[50]  = { 
		1, 2, 3, 4, 5, 
		6, 7, 8, 9,10, 
	 	11, 12, 13, 14, 15, 
		2, 6, 2, 3, 8, 
		4, 1, 6, 2, 1, 
		4, 1, 2, 5, 5, 
		2, 2, 3, 8, 7, 
		2, 4, 8, 1, 4, 
		1, 2, 3, 2, 2, 
		5, 4, 2, 3, 3
	}; 
	
	Matrix m(10, 5, input); 

	auto mr = m.get_column(2); 
	BOOST_CHECK_EQUAL(mr.size(), 10); 

	BOOST_CHECK_EQUAL(mr[0], 3); 
	BOOST_CHECK_EQUAL(mr[1], 8); 
	BOOST_CHECK_EQUAL(mr[2], 13); 
	BOOST_CHECK_EQUAL(mr[3], 2); 
	BOOST_CHECK_EQUAL(mr[4], 6); 

	BOOST_CHECK_EQUAL(mr[5], 2); 
	BOOST_CHECK_EQUAL(mr[6], 3); 
	BOOST_CHECK_EQUAL(mr[7], 8); 
	BOOST_CHECK_EQUAL(mr[8], 3); 
	BOOST_CHECK_EQUAL(mr[9], 2); 	

	
	auto b = mr.begin(); 

	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 8); ++b; 
	BOOST_CHECK_EQUAL(*b, 13); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 
	BOOST_CHECK_EQUAL(*b, 6); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 
	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 8); ++b; 
	BOOST_CHECK_EQUAL(*b, 3); ++b; 
	BOOST_CHECK_EQUAL(*b, 2); ++b; 

	BOOST_CHECK(b == mr.end()); 

	m.set_column(4, mr); 


	auto mr3 = m.get_column(4); 
	BOOST_CHECK_EQUAL(mr3.size(), 10); 

	BOOST_CHECK_EQUAL(mr3[0], 3); 
	BOOST_CHECK_EQUAL(mr3[1], 8); 
	BOOST_CHECK_EQUAL(mr3[2], 13); 
	BOOST_CHECK_EQUAL(mr3[3], 2); 
	BOOST_CHECK_EQUAL(mr3[4], 6); 

	BOOST_CHECK_EQUAL(mr3[5], 2); 
	BOOST_CHECK_EQUAL(mr3[6], 3); 
	BOOST_CHECK_EQUAL(mr3[7], 8); 
	BOOST_CHECK_EQUAL(mr3[8], 3); 
	BOOST_CHECK_EQUAL(mr3[9], 2); 	
}


