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

#include <cmath> 

#include <mia/internal/autotest.hh>
#include <mia/core/fastica.hh>
#include <mia/core/cmdlineparser.hh>

#include <mia/core/gsl_matrix_vector_ops.hh>

#include <gsl/gsl_blas.h>

using gsl::Matrix; 
using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_fastica_symm ) 
{
	// create the components and the mixing matrix 

	const double c[] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  
			    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
			    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};

	Matrix in_ics(3, 13, c); 
	const int steps = 101; 

	Matrix in_mixing_matrix(steps, 3, false);

	auto c0 = in_mixing_matrix.get_column(0);
	auto c1 = in_mixing_matrix.get_column(1);
	auto c2 = in_mixing_matrix.get_column(2);

	// sin(x) ; cos(x); sin(2x)
	for ( int i = 0; i < steps; ++i) {
		cvdebug() << "init row " << i << "\n"; 
		double x = (M_PI * (i - (steps- 1)/2)) / steps;
		c0[i] = sin(x); 
		c1[i] = cos(x); 
		c2[i] = sin(2 * x);
	}
	
	Matrix mix = in_mixing_matrix * in_ics; 
	

	FastICA ica(3);

	ica.set_approach(FastICA::appr_symm); 
 	ica.set_epsilon (1e-10); 
	ica.set_finetune(true); 
	ica.set_nonlinearity(produce_fastica_nonlinearity("pow3")); 
	BOOST_CHECK(ica.separate(mix)); 
	
	
	const gsl::Matrix& out_mixing_matrix = ica.get_mixing_matrix();
	const gsl::Matrix& out_ics = ica.get_independent_components();

	// the mixes should be close to orthogonal 
	
	for (unsigned int c = 1; c < out_mixing_matrix.cols(); ++c) {
		auto col_a = out_mixing_matrix.get_column(c); 
		double na = 0.0; 
		gsl_blas_ddot(col_a, col_a, &na); 
		for (unsigned int c1 = 0; c1 < c; ++c1) {
			auto col_b = out_mixing_matrix.get_column(c1); 
			auto dot = 1.0; 
			double nb = 0.0; 
			gsl_blas_ddot(col_b, col_b, &nb); 
			gsl_blas_ddot(col_a, col_b, & dot); 
			BOOST_CHECK_SMALL(dot, 0.1 * na * nb); 
		}
	}


	BOOST_CHECK_EQUAL(out_mixing_matrix.rows(), in_mixing_matrix.rows()); 
	BOOST_CHECK_EQUAL(out_mixing_matrix.cols(), in_mixing_matrix.cols()); 
	BOOST_CHECK_EQUAL(out_ics.rows(), in_ics.rows()); 
	BOOST_CHECK_EQUAL(out_ics.cols(), in_ics.cols()); 
	
	// create the remix and test it against the input mix
	

	Matrix remix = out_mixing_matrix * out_ics;
	Matrix delta = remix - mix; 
	
	for(auto id = delta.begin(); id != delta.end(); ++id) {
		BOOST_CHECK_SMALL(*id, 1e-10); 
	}
	
}



BOOST_AUTO_TEST_CASE ( test_fastica_defl ) 
{
	// create the components and the mixing matrix 

	const double c[] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  
			    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
			    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};

	Matrix in_ics(3, 13, c); 
	const int steps = 101; 

	Matrix in_mixing_matrix(steps, 3, false);

	auto c0 = in_mixing_matrix.get_column(0);
	auto c1 = in_mixing_matrix.get_column(1);
	auto c2 = in_mixing_matrix.get_column(2);

	// sin(x) ; cos(x); sin(2x)
	for ( int i = 0; i < steps; ++i) {
		cvdebug() << "init row " << i << "\n"; 
		double x = (M_PI * (i - (steps- 1)/2)) / steps;
		c0[i] = sin(x); 
		c1[i] = cos(x); 
		c2[i] = sin(2 * x);
	}
	
	Matrix mix = in_mixing_matrix * in_ics; 
	

	FastICA ica(3);

	ica.set_approach(FastICA::appr_defl); 
 	ica.set_epsilon (1e-10); 
	ica.set_finetune(true); 
	ica.set_stabilization(true); 
	ica.set_nonlinearity(produce_fastica_nonlinearity("pow3")); 
	BOOST_CHECK(ica.separate(mix)); 
	
	
	const gsl::Matrix& out_mixing_matrix = ica.get_mixing_matrix();
	const gsl::Matrix& out_ics = ica.get_independent_components();

	// the mixes should be close to orthogonal 
	
	for (unsigned int c = 1; c < out_mixing_matrix.cols(); ++c) {
		auto col_a = out_mixing_matrix.get_column(c); 
		double na = 0.0; 
		gsl_blas_ddot(col_a, col_a, &na); 
		for (unsigned int c1 = 0; c1 < c; ++c1) {
			auto col_b = out_mixing_matrix.get_column(c1); 
			auto dot = 1.0; 
			double nb = 0.0; 
			gsl_blas_ddot(col_b, col_b, &nb); 
			gsl_blas_ddot(col_a, col_b, & dot); 
			BOOST_CHECK_SMALL(dot, 0.1 * na * nb); 
		}
	}


	BOOST_CHECK_EQUAL(out_mixing_matrix.rows(), in_mixing_matrix.rows()); 
	BOOST_CHECK_EQUAL(out_mixing_matrix.cols(), in_mixing_matrix.cols()); 
	

	BOOST_CHECK_EQUAL(out_ics.rows(), in_ics.rows()); 
	BOOST_CHECK_EQUAL(out_ics.cols(), in_ics.cols()); 

	Matrix remix = out_mixing_matrix * out_ics;
	Matrix delta = remix - mix; 
	
	for(auto id = delta.begin(); id != delta.end(); ++id) {
		BOOST_CHECK_SMALL(*id, 1e-10); 
	}
	
}
