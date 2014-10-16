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

#include <gsl++/matrix_vector_ops.hh>

using gsl::Matrix; 
using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_fastica_symm ) 
{
	// create the components and the mixing matrix 

	const double c[] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  
			    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
			    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};

	Matrix in_ics(3, 13, c); 

	Matrix in_mixing_matrix(21, 3, false);

	auto c0 = in_mixing_matrix.get_column(0);
	auto c1 = in_mixing_matrix.get_column(1);
	auto c2 = in_mixing_matrix.get_column(2);

	// sin(x) ; cos(x); sin(2x)
	for ( int i = 0; i < 21; ++i) {
		cvdebug() << "init row " << i << "\n"; 
		double x = M_PI * (i - 10.0) / 21.0;
		c0[i] = sin(x); 
		c1[i] = cos(x); 
		c2[i] = sin(2 * x);
	}
	
	Matrix mix = in_mixing_matrix * in_ics; 
	

	FastICA ica(mix);

	ica.set_approach(FastICA::appr_symm); 
	ica.set_nrof_independent_components (3); 
	
	BOOST_CHECK(ica.separate()); 
	
	
	const gsl::Matrix& out_mixing_matrix = ica.get_mixing_matrix();
	const gsl::Matrix& out_ics = ica.get_independent_components();

	BOOST_CHECK_EQUAL(out_mixing_matrix.rows(), in_mixing_matrix.rows()); 
	BOOST_CHECK_EQUAL(out_mixing_matrix.cols(), in_mixing_matrix.cols()); 
	
	for (unsigned int r = 0; r < out_mixing_matrix.rows(); ++r) 
		for (unsigned int c = 0; c < out_mixing_matrix.cols(); ++c) {
			cvdebug() << ""<< r << "x" << c <<": have "
				  << out_mixing_matrix(r,c) << " expect " 
				  << in_mixing_matrix(r,c) << "\n"; 
			BOOST_CHECK_CLOSE(out_mixing_matrix(r,c), in_mixing_matrix(r,c), 0.1); 
		}

	BOOST_CHECK_EQUAL(out_ics.rows(), in_ics.rows()); 
	BOOST_CHECK_EQUAL(out_ics.cols(), in_ics.cols()); 
	
	for (unsigned int r = 0; r < out_ics.rows(); ++r) 
		for (unsigned int c = 0; c < out_ics.cols(); ++c) {
			cvdebug() << ""<< r << "x" << c <<": have "
				  << out_ics(r,c) << " expect " 
				  << in_ics(r,c) << "\n"; 

			BOOST_CHECK_CLOSE(out_ics(r,c), in_ics(r,c), 0.1); 
		}
	
}


