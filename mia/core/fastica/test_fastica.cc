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

#include <cmath> 

#include <mia/internal/autotest.hh>
#include <mia/core/fastica/fastica.hh>

#include <mia/core/gsl_matrix_vector_ops.hh>

#include <gsl/gsl_blas.h>

using gsl::Matrix; 
using namespace mia;

using std::vector;
using std::invalid_argument; 

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

	ica.set_approach(CIndepCompAnalysis::appr_symm); 
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

	ica.set_approach(CIndepCompAnalysis::appr_defl); 
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

//

BOOST_AUTO_TEST_CASE( test_mia_mixing_ica_without_mean )
{
	const unsigned  time_steps = 5;
	const unsigned  elms = 10;
	const unsigned  nica = 3;

	const double init_mix[nica * time_steps] = {
		1, 2, 3, 4, 5,
		1, 3, 5, 3, 1,
		1, 1, 1, 1, 1
	};

	const double init_ic[nica * elms] = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 1, 2,-1, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 4, 3, 2, 0
	};

	double test_rows[time_steps][elms] = {
		{ 1, 1, 2, 1, 2,  0, 4, 3, 2, 1 } ,
		{ 2, 2, 3, 3, 6, -2, 4, 3, 2, 2 } ,
		{ 3, 3, 4, 5,10, -4, 4, 3, 2, 3 } ,
		{ 4, 4, 5, 3, 6, -2, 4, 3, 2, 4 } ,
		{ 5, 5, 6, 1, 2,  0, 4, 3, 2, 5 }
	};


	gsl::Matrix mix(nica, time_steps,  init_mix);
	gsl::Matrix ic(nica, elms, init_ic);
	vector<double> mean(time_steps, 0.0);

	CICAAnalysisMIA ica(ic, mix.transposed(), mean);
	
	for (unsigned  i = 0; i < time_steps; ++i) {
		vector<float> mixed = ica.get_mix(i);
		BOOST_CHECK_EQUAL(mixed.size(), elms); 
		for (unsigned k = 0; k < elms; ++k) {
			if (mixed[k] != test_rows[i][k])
				cverr() << i << ", " << k << " expect " << test_rows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_rows[i][k], 0.001);
		}
	}
}


BOOST_AUTO_TEST_CASE( test_mia_mixing_ica_with_skip )
{
	const int rows = 5;
	const int elms = 10;
	const int nica = 3;
	const size_t skipnr = 0;

	const double init_mix[nica * rows] = {
		1, 2, 3, 4, 5,
		1, 3, 5, 3, 1,
		1, 1, 1, 1, 1
	};

	const double init_ic[rows * elms] = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 1, 2,-1, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 4, 3, 2, 0
	};

	double test_miarows[rows][elms] = {
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 5,10, -4, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 }
	};


	gsl::Matrix mix(nica, rows, init_mix);
	gsl::Matrix ic(nica, elms, init_ic);
	vector<double> mean(rows, 0.0);
    
	CICAAnalysisMIA ica(ic, mix.transposed(), mean);
	CIndepCompAnalysis::IndexSet skip;
	skip.insert(skipnr);

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_incomplete_mix(i, skip);
		for (int k = 0; k < elms; ++k) {
			if (mixed[k] != test_miarows[i][k])
				cverr() << i << ", " << k << " expect " << test_miarows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_miarows[i][k], 0.001);
		}
	}
}

BOOST_AUTO_TEST_CASE( test_miapartial_ica_mix )
{
	const int rows = 5;
	const int elms = 10;
	const int nica = 3;

	const double init_mix[nica * rows] = {
		1, 2, 3, 4, 5,
		1, 3, 5, 3, 1,
		1, 1, 1, 1, 1
	};

	const double init_ic[rows * elms] = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 1, 2,-1, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 4, 3, 2, 0
	};

	double test_miarows[rows][elms] = {
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 5,10, -4, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 }
	};


	gsl::Matrix mix(nica, rows, init_mix);
	gsl::Matrix ic(nica, elms, init_ic);
	vector<double> mean(rows, 0.0);

	CICAAnalysisMIA ica(ic, mix.transposed(), mean);
	CIndepCompAnalysis::IndexSet components;
	components.insert(1);
	components.insert(2);

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_partial_mix(i, components);
		for (int k = 0; k < elms; ++k) {
			if (mixed[k] != test_miarows[i][k])
				cverr() << i << ", " << k << " expect " << test_miarows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_miarows[i][k], 0.001);
		}
	}
}

BOOST_AUTO_TEST_CASE( test_miadelta_ica_mix )
{
	const int rows = 5;
	const int elms = 10;
	const int nica = 3;

	const double init_mix[nica * rows] = {
		1, 2, 3, 4, 5,
		1, 3, 5, 3, 1,
		1, 1, 1, 1, 1
	};

	const double init_ic[rows * elms] = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 1, 2,-1, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 4, 3, 2, 0
	};

	double test_miarows[elms] = {
		 -1, -1, 0, 1, 2,  0, 4, 3, 2, -1 ,
	};


	gsl::Matrix mix(nica, rows, init_mix);
	gsl::Matrix ic(nica, elms, init_ic);
	vector<double> mean(rows, 0.0);

	CICAAnalysisMIA ica(ic, mix.transposed(), mean);
	CIndepCompAnalysis::IndexSet plus_components;
	plus_components.insert(1);
	plus_components.insert(2);

	CIndepCompAnalysis::IndexSet minus_components;
	minus_components.insert(0);

	vector<float> mixed = ica.get_delta_feature(plus_components, minus_components);
	for (int k = 0; k < elms; ++k) {
		if (mixed[k] != test_miarows[k])
			cverr()  << k << " expect " << test_miarows[k]
				<< " get " <<  mixed[k] << "\n";
		BOOST_CHECK_CLOSE(mixed[k], test_miarows[k], 0.001);
	}
}


BOOST_AUTO_TEST_CASE( test_miaica_with_zero_mean )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1, -1,  -2,  -1,  2, -2, 6, -3, -1, 1 } ,
		{ 2, -2,  -3,  -3,  6, -4, 6, -3, -1, 2 } ,
		{ 3, -3,  -4,  -5, 10, -6, 6, -3, -1, 3 } ,
		{ 4, -4,  -5,  -3,  6, -4, 6, -3, -1, 4 } ,
		{ 5, -5,  -6,  -1,  2, -2, 6, -3, -1, 5 }
	};
	
	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);


	ica.run(3, vector<vector<float> >());

	BOOST_CHECK_EQUAL(ica.get_ncomponents(), 3u); 
	
	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			BOOST_CHECK_CLOSE(mixed[k], data_rows[i][k], 0.001);
		}

	}
}


BOOST_AUTO_TEST_CASE( test_miaica_with_some_mean )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 4, -4,  -5,  -3,  6, -4,  6, -3, -1, 4 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};
	
	CICAAnalysisMIA ica; 
	ica.initialize(rows, elms);
	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);
	ica.run(3, vector<vector<float> >());
	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			BOOST_CHECK_CLOSE(mixed[k], data_rows[i][k], 0.001);
		}
	}
}

BOOST_AUTO_TEST_CASE( test_miaica_with_some_mean_unknown )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};

	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);


	ica.run(4, vector<vector<float> >());

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			BOOST_CHECK_CLOSE(mixed[k], data_rows[i][k], 0.001);
		}

	}
}

BOOST_AUTO_TEST_CASE( test_miaica_with_some_mean_unknown_SYMM )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};

	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);
	ica.set_approach(CIndepCompAnalysis::appr_symm);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);


	ica.run(4, vector<vector<float> >());

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			BOOST_CHECK_CLOSE(mixed[k], data_rows[i][k], 0.001);
		}

	}
}



BOOST_AUTO_TEST_CASE( test_miaica_with_some_mean_unknown_normalized_mix )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};
	
	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);


	ica.run(4, vector<vector<float> >());
	vector<float>  mean = ica.normalize_Mix();


	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k)
			BOOST_CHECK_CLOSE(mean[k] + mixed[k], data_rows[i][k], 0.001);
	}
}


BOOST_AUTO_TEST_CASE( test_miaica_with_some_mean_unknown_normalized )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};

	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);


	ica.run(4, vector<vector<float> >());

	ica.normalize_ICs();

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			BOOST_CHECK_CLOSE(mixed[k], data_rows[i][k], 0.001);
		}

	}
}

BOOST_AUTO_TEST_CASE( test_miaica_saftey_against_stupid )
{
	const int rows = 5;
	const int elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};

	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);

	BOOST_CHECK_THROW(ica.get_feature_row(0), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix_series(0), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix(0), invalid_argument);

}

BOOST_AUTO_TEST_CASE( test_miaica_access_failtures )
{
	const int ncomponents = 4;
	const size_t rows = 5;
	const size_t elms = 10;
	double data_rows[rows][elms] = {
		{ 1.1, -0.9,  -1.9,  -0.9,  2.1, -1.9,  6.1, -2.9, -0.9, 1.1 },
		{ 2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3 },
		{ 2, -4,  -5,  -6,  9, -7,  5, -4, -2, 2 },
		{ 6, -2,  -5,  -13,  6, -4,  6, -3, -11, 14 },
		{ 1, -9,  -10,  -5,  -2, -6,  2, -7, -5, 1 }
	};

	CICAAnalysisMIA ica;
	ica.initialize(rows, elms);

	for (size_t i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);
	ica.run(4, vector<vector<float> >());

	BOOST_CHECK_THROW(ica.get_feature_row(ncomponents), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix_series(ncomponents), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix(rows), invalid_argument);

	BOOST_CHECK_NO_THROW( ica.get_feature_row(ncomponents - 1));
	BOOST_CHECK_NO_THROW( ica.get_mix_series(ncomponents - 1));
	BOOST_CHECK_NO_THROW( ica.get_mix(rows-1));

	BOOST_CHECK_EQUAL(ica.get_feature_row(0).size(), elms);
	BOOST_CHECK_EQUAL(ica.get_mix_series(0).size(), rows);
	BOOST_CHECK_EQUAL(ica.get_mix(0).size(), elms);

}
