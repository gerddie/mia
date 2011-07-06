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

#include <mia/internal/autotest.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/core/splinekernel.hh>
#include <gsl/gsl_linalg.h>
#include <gsl++/vector.hh>
#include <gsl++/matrix.hh>


BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_needed ) 
{
	std::vector<int> index  = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; 
	std::vector<int> result = { 1, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 1 }; 
	std::vector<double> weights(12); 
	
	CMirrorOnBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}

BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_notneeded ) 
{
	std::vector<int> index  = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	std::vector<int> result = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	std::vector<double> weights(9); 
	
	CMirrorOnBoundary bc(9);
	BOOST_CHECK(bc.apply(index, weights)); 

	for (int i = 0; i < 9; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}


BOOST_AUTO_TEST_CASE( test_zero_boundary_needed ) 
{
	std::vector<int> index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11}; 
	std::vector<int> result =     { 0, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0}; 
	std::vector<double> weights = { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13}; 
	std::vector<double> test    = { 0, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0}; 

	CZeroBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
		BOOST_CHECK_EQUAL(weights[i], test[i]); 
	}
}

BOOST_AUTO_TEST_CASE( test_CRepeatBoundary_needed ) 
{
	std::vector<int> index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7}; 
	std::vector<int> result =     { 0, 0, 1, 2, 3, 4, 5, 5, 5}; 
	std::vector<double> weights = { 1, 2, 3, 4, 5, 6, 7, 6, 8}; 

	CRepeatBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
	}
}

class BoundaryFixture {
protected: 
	void run(std::vector<double> f, const CBoundaryCondition& bc, PSplineKernel kernel); 
}; 


void BoundaryFixture::run(std::vector<double> f, const CBoundaryCondition& bc, PSplineKernel kernel)
{
	vector<double> weights(kernel->size()); 
	vector<int>    indices(kernel->size()); 

	auto m_A = gsl::Matrix(f.size(), f.size(),  true);
	auto m_tau = gsl::DoubleVector(f.size() ); 

	for(size_t i = 0; i < f.size(); ++i) {
		(*kernel)(i, weights, indices);
		bc.apply(indices, weights);
		
		for(int j = 0; j < indices.size(); ++j) { 
			cvdebug() << i << " " << j << " " << indices[j] << " " << weights[j]<<"\n"; 
			double v = m_A(i, indices[j]); 
			m_A.set(i, indices[j], v + weights[j]);
		}
	}
	gsl_linalg_QR_decomp(m_A, m_tau); 
	

	gsl::DoubleVector coefs(f.size()); 
	gsl::DoubleVector residual(f.size()); 
	gsl::DoubleVector input(f.size()); 
	copy(f.begin(), f.end(), input.begin()); 

	gsl_linalg_QR_lssolve (m_A, m_tau, input, coefs, residual); 

	bc.filter_line(f, kernel->get_poles()); 
	double old_delta = 1.0; 
	for (int i = 0; i < f.size(); ++i) {
		BOOST_CHECK_CLOSE(f[i], coefs[i], 0.01); 
		double delta = f[i] - coefs[i]; 
		cvdebug() << "delta = " << delta  << ", q=" << delta / old_delta  << "\n"; 
		old_delta  =  delta; 
	}
}


BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat1, BoundaryFixture ) 
{
	std::vector<double> f = { 1, 2, 3, 4, 5, 6, 7, 8, 9}; 
//	std::vector<double> f = { 9, 9, 9, 9, 9, 9, 9, 9, 9}; 

	CRepeatBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat0, BoundaryFixture ) 
{
	std::vector<double> f = { 9, 8, 7, 6, 5, 4, 3, 2, 1}; 
//	std::vector<double> f = { 9, 9, 9, 9, 9, 9, 9, 9, 9}; 

	CRepeatBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat3, BoundaryFixture ) 
{
	std::vector<double> f = { 11, 11, 3, 7, 5, 6, 2, 11, 11}; 
//	std::vector<double> f = { 9, 9, 9, 9, 9, 9, 9, 9, 9}; 

	CRepeatBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat4, BoundaryFixture ) 
{
	std::vector<double> f = { 1, 5, 3, 7, 5, 6, 2, 6, 11}; 
//	std::vector<double> f = { 9, 9, 9, 9, 9, 9, 9, 9, 9}; 

	CRepeatBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat2, BoundaryFixture ) 
{
	std::vector<double> f = { 9, 9, 9, 9, 9, 9, 9, 9, 9}; 

	CRepeatBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 
}


BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_mirror , BoundaryFixture ) 
{
	std::vector<double> f = { 9, 9, 9, 7, 5, 6, 1, 1, 1}; 

	CMirrorOnBoundary bc(f.size());
	auto kernel = produce_spline_kernel("bspline:d=3"); 

	run(f, bc, kernel); 

}

