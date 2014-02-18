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

#include <stdexcept>
#include <cmath>
#include <iomanip>

#include <mia/core/ica.hh>


using namespace std;
using namespace mia;


template <typename T>
ostream& operator << (ostream& os, const vector<T>& v)
{
	os << "[";
	for(typename vector<T>::const_iterator i = v.begin(); i != v.end(); ++i)
		os << setw(9)<<*i << ",";
	os << "]";
	return os;
}

BOOST_AUTO_TEST_CASE( test_mixing_ica_without_mean )
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

	double test_rows[rows][elms] = {
		{ 1, 1, 2, 1, 2,  0, 4, 3, 2, 1 } ,
		{ 2, 2, 3, 3, 6, -2, 4, 3, 2, 2 } ,
		{ 3, 3, 4, 5,10, -4, 4, 3, 2, 3 } ,
		{ 4, 4, 5, 3, 6, -2, 4, 3, 2, 4 } ,
		{ 5, 5, 6, 1, 2,  0, 4, 3, 2, 5 }
	};


	itpp::mat mix(init_mix, rows, nica, false);
	itpp::mat ic(init_ic,  rows, elms, true);
	vector<float> mean(rows, 0.0);

	CICAAnalysis ica(ic, mix, mean);

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_mix(i);
		for (int k = 0; k < elms; ++k) {
			if (mixed[k] != test_rows[i][k])
				cverr() << i << ", " << k << " expect " << test_rows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_rows[i][k], 0.001);
		}
	}
}


BOOST_AUTO_TEST_CASE( test_mixing_ica_with_skip )
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

	double test_rows[rows][elms] = {
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 5,10, -4, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 }
	};


	itpp::mat mix(init_mix, rows, nica, false);
	itpp::mat ic(init_ic,  rows, elms, true);
	vector<float> mean(rows, 0.0);

	CICAAnalysis ica(ic, mix, mean);
	CICAAnalysis::IndexSet skip;
	skip.insert(skipnr);

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_incomplete_mix(i, skip);
		for (int k = 0; k < elms; ++k) {
			if (mixed[k] != test_rows[i][k])
				cverr() << i << ", " << k << " expect " << test_rows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_rows[i][k], 0.001);
		}
	}
}

BOOST_AUTO_TEST_CASE( test_partial_ica_mix )
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

	double test_rows[rows][elms] = {
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 5,10, -4, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 3, 6, -2, 4, 3, 2, 0 } ,
		{ 0, 0, 1, 1, 2,  0, 4, 3, 2, 0 }
	};


	itpp::mat mix(init_mix, rows, nica, false);
	itpp::mat ic(init_ic,  rows, elms, true);
	vector<float> mean(rows, 0.0);

	CICAAnalysis ica(ic, mix, mean);
	CICAAnalysis::IndexSet components;
	components.insert(1);
	components.insert(2);

	for (int i = 0; i < rows; ++i) {
		vector<float> mixed = ica.get_partial_mix(i, components);
		for (int k = 0; k < elms; ++k) {
			if (mixed[k] != test_rows[i][k])
				cverr() << i << ", " << k << " expect " << test_rows[i][k]
					<< " get " <<  mixed[k] << "\n";
			BOOST_CHECK_CLOSE(mixed[k], test_rows[i][k], 0.001);
		}
	}
}

BOOST_AUTO_TEST_CASE( test_delta_ica_mix )
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

	double test_rows[elms] = {
		 -1, -1, 0, 1, 2,  0, 4, 3, 2, -1 ,
	};


	itpp::mat mix(init_mix, rows, nica, false);
	itpp::mat ic(init_ic,  rows, elms, true);
	vector<float> mean(rows, 0.0);

	CICAAnalysis ica(ic, mix, mean);
	CICAAnalysis::IndexSet plus_components;
	plus_components.insert(1);
	plus_components.insert(2);

	CICAAnalysis::IndexSet minus_components;
	minus_components.insert(0);

	vector<float> mixed = ica.get_delta_feature(plus_components, minus_components);
	for (int k = 0; k < elms; ++k) {
		if (mixed[k] != test_rows[k])
			cverr()  << k << " expect " << test_rows[k]
				<< " get " <<  mixed[k] << "\n";
		BOOST_CHECK_CLOSE(mixed[k], test_rows[k], 0.001);
	}
}


BOOST_AUTO_TEST_CASE( test_ica_with_zero_mean )
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

	CICAAnalysis ica(rows, elms);

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


BOOST_AUTO_TEST_CASE( test_ica_with_some_mean )
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

	CICAAnalysis ica(rows, elms);
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

BOOST_AUTO_TEST_CASE( test_ica_with_some_mean_unknown )
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

	CICAAnalysis ica(rows, elms);

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

BOOST_AUTO_TEST_CASE( test_ica_with_some_mean_unknown_SYMM )
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

	CICAAnalysis ica(rows, elms);
	ica.set_approach(FICA_APPROACH_SYMM); 

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



BOOST_AUTO_TEST_CASE( test_ica_with_some_mean_unknown_normalized_mix )
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

	CICAAnalysis ica(rows, elms);

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


BOOST_AUTO_TEST_CASE( test_ica_with_some_mean_unknown_normalized )
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

	CICAAnalysis ica(rows, elms);

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

BOOST_AUTO_TEST_CASE( test_ica_saftey_against_stupid )
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

	CICAAnalysis ica(rows, elms);

	for (int i = 0; i < rows; ++i)
		ica.set_row(i, data_rows[i], data_rows[i] + elms);

	BOOST_CHECK_THROW(ica.get_feature_row(0), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix_series(0), invalid_argument);
	BOOST_CHECK_THROW(ica.get_mix(0), invalid_argument);

}

BOOST_AUTO_TEST_CASE( test_ica_access_failtures )
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

	CICAAnalysis ica(rows, elms);

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

/**
 This test needs more data to work properly
BOOST_AUTO_TEST_CASE( test_autorun )
{
	const int comps = 3;
	const int rows = 10;
	const int elms = 40;
	double ic_rows[comps * elms] =
		{ 1.1, -0.9,  -1.9,  0.9,  2.1, 1.9,  6.1, -2.9,  0.9, 1.1,
		  1.1,  1.9,  -1.1,  2.9,  3.1, 1.9,  0.1,  2.9,  1.9, 2.1,
		  2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3,
		  2.3, -2.7,   2.2,   2.6,  6.4, -3.8,  6.2, -2.5, -0.6, 2.2,

		  2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3,
		  2.3, -2.7,  -2.2,  -2.6,  6.4, -3.8,  6.2, -2.5, -0.6, 2.2,
		  2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3,
		  2.3, -2.7,  -2.2,  -2.6,  6.4, -3.8,  6.2, -2.5, -0.6, 2.2 ,

		  2.3, -1.7,  -2.7,  -2.7,  6.3, -3.7,  6.3, -2.7, -0.7, 2.3,
		  2,   4.4,    -5,    6,    9,   -7,    5,   -4,   2,   2,
		  1.1,  1.9,  -1.1,  2.9,  3.1, 1.9,  0.1,  2.9,  1.9, 2.1,
		  2.1, 4.1,  -5.2,  6.4,  -9.2, -7.1,  -5.3, 4.4,  -2.1,  2.1
	};

	CICAAnalysis ica(rows, elms);

	for (int i = 0; i < rows; ++i) {
		vector <float> input(elms);
		for (int j = 0; j < elms; ++j)
			for (int k = 0; k < comps; ++k)
				input[j] += ic_rows[k * elms + j] * cos(i  +  k);

		ica.set_row(i, input.begin(), input.end());
	}
	ica.run_auto(6,2,0.9);
	BOOST_CHECK_EQUAL(ica.get_ncomponents(), comps);


}


*/
