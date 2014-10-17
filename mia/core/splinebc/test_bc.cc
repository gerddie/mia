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

#include <mia/internal/autotest.hh>
#include <mia/core/splinebc/bc.hh>

#include <iomanip>
#include <mia/core/splinekernel.hh>
#include <gsl/gsl_linalg.h>
#include <gsl++/vector.hh>
#include <gsl++/matrix.hh>

NS_MIA_USE
using std::vector; 
using std::setprecision; 
using std::invalid_argument; 

BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_needed ) 
{
	CSplineKernel::VIndex index  = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; 
	CSplineKernel::VIndex result = { 1, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 1 }; 
	CSplineKernel::VWeight weights(12); 
	
	CMirrorOnBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}

BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_needed_large ) 
{
	CSplineKernel::VIndex index  = {-32000, 32001}; 
	CSplineKernel::VIndex result = { 4, 3}; 
	CSplineKernel::VWeight weights(2); 
	
	CMirrorOnBoundary bc(10);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (size_t i = 0; i < index.size(); ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}


BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_notneeded ) 
{
	CSplineKernel::VIndex index  = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	CSplineKernel::VIndex result = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	CSplineKernel::VWeight weights(9); 
	
	CMirrorOnBoundary bc(9);
	BOOST_CHECK(bc.apply(index, weights)); 

	for (int i = 0; i < 9; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}


BOOST_AUTO_TEST_CASE( test_zero_boundary_needed ) 
{
	CSplineKernel::VIndex index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11}; 
	CSplineKernel::VIndex result =     { 0, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0}; 
	CSplineKernel::VWeight weights = { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13}; 
	CSplineKernel::VWeight test    = { 0, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0}; 

	CZeroBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
		BOOST_CHECK_EQUAL(weights[i], test[i]); 
	}
}

BOOST_AUTO_TEST_CASE( test_CRepeatBoundary_needed ) 
{
	CSplineKernel::VIndex index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7}; 
	CSplineKernel::VIndex result =     { 0, 0, 1, 2, 3, 4, 5, 5, 5}; 
	CSplineKernel::VWeight weights = { 1, 2, 3, 4, 5, 6, 7, 6, 8}; 

	CRepeatBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (size_t i = 0; i < index.size(); ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
	}
}

class BoundaryFixture {
protected: 
	vector<double>  run(std::vector<double> f, const CSplineBoundaryCondition& bc, PSplineKernel kernel); 
	void prepare_and_run(CSplineBoundaryCondition& bc, PSplineKernel kernel); 
}; 

vector<double> BoundaryFixture::run(std::vector<double> f, const CSplineBoundaryCondition& bc, PSplineKernel kernel)
{
	CSplineKernel::VWeight weights(kernel->size()); 
	CSplineKernel::VIndex    indices(kernel->size());
	CSplineKernel::VWeight orig(f); 

	auto m_A = gsl::Matrix(f.size(), f.size(),  true);
	auto m_tau = gsl::Vector(f.size(), false ); 

	for(size_t i = 0; i < f.size(); ++i) {
		(*kernel)(i, weights, indices);
		bc.apply(indices, weights);
		
		for(size_t j = 0; j < indices.size(); ++j) { 
			cvinfo() << "A(" << i << ", " << indices[j] << ")=" << weights[j] << "\n"; 
			double v = m_A(i, indices[j]); 
			m_A.set(i, indices[j], v + weights[j]);
		}
	}
	gsl_linalg_QR_decomp(m_A, m_tau); 
	

	gsl::Vector coefs(f.size(), false); 
	gsl::Vector residual(f.size(), false); 
	gsl::Vector input(f.size(), false); 
	copy(f.begin(), f.end(), input.begin()); 

	gsl_linalg_QR_lssolve (m_A, m_tau, input, coefs, residual); 

	bc.filter_line(f, kernel->get_poles()); 
	double old_delta = 1.0; 
	for (size_t i = 0; i < f.size(); ++i) {
		BOOST_CHECK_CLOSE(f[i], coefs[i], 0.01); 
		double delta = f[i] - coefs[i]; 
		cvdebug() << "i=" << i << ", delta = " << delta  << ", q=" << delta / old_delta  << "\n"; 
		old_delta  =  delta; 
	}

	transform(f.begin(), f.end(), coefs.begin(), f.begin(), [](double x, double y){return x - y;}); 
	return f ; 
}

void BoundaryFixture::prepare_and_run(CSplineBoundaryCondition& bc, PSplineKernel kernel)
{
	std::vector<double> f = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}; 

	bc.set_width(f.size());

	for (int j = -1; j < 12; ++j) { 
		std::vector<double> t = f; 
		if (j >= 0) 
			t[j] = 1; 
		cverb << setprecision(10); 
		cvinfo() << "values[" << j << "]=" << run(t, bc, kernel) << "\n";; 
	}
}



BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat_4  , BoundaryFixture ) 
{
	CRepeatBoundary bc; 
	BOOST_CHECK_THROW(prepare_and_run(bc, produce_spline_kernel("bspline:d=4")), 
			  invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat_5  , BoundaryFixture ) 
{
	CRepeatBoundary bc; 
	BOOST_CHECK_THROW(prepare_and_run(bc, produce_spline_kernel("bspline:d=5")), 
			  invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat_2  , BoundaryFixture ) 
{
	CRepeatBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=2")); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_repeat_3  , BoundaryFixture ) 
{
	CRepeatBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=3")); 
}


BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_mirror_3 , BoundaryFixture ) 
{
	CMirrorOnBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=3")); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_mirror_4 , BoundaryFixture ) 
{
	CMirrorOnBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=4")); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_mirror_5 , BoundaryFixture ) 
{
	CMirrorOnBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=5")); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_mirror_2 , BoundaryFixture ) 
{
	CMirrorOnBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=2")); 
}


BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_zero_3 , BoundaryFixture ) 
{
	CZeroBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=3")); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_zero_4 , BoundaryFixture ) 
{
	CZeroBoundary bc; 
	BOOST_CHECK_THROW(prepare_and_run(bc, produce_spline_kernel("bspline:d=5")), 
			  invalid_argument); 
}

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_zero_5 , BoundaryFixture ) 
{
	CZeroBoundary bc; 
	BOOST_CHECK_THROW(prepare_and_run(bc, produce_spline_kernel("bspline:d=5")), 
			  invalid_argument); 
}

#if SUPPORT_MORE_THEN_ONE_POLE_ON_ZERO_BOUNDARY_CONDITION
BOOST_FIXTURE_TEST_CASE( test_CZeroBoundary_coefs_large_5 , BoundaryFixture ) 
{

	std::vector<double> f(30); 
	cverb << setprecision(10); 
	fill(f.begin(), f.end(), 2.0); 
	CZeroBoundary bc(30); 
	cvinfo() << run(f, bc, produce_spline_kernel("bspline:d=5")) << "\n"; 
}
#endif

BOOST_FIXTURE_TEST_CASE( test_CRepeatBoundary_coefs_zero_2, BoundaryFixture ) 
{

	CZeroBoundary bc; 
	prepare_and_run(bc, produce_spline_kernel("bspline:d=2"));

}


