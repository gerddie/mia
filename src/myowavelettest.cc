/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#define VSTREAM_DOMAIN "wavelet"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <gsl++/wavelet.hh>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/slopestatistics.hh>
#include <mia/core/waveletslopeclassifier.hh>

using namespace std; 

NS_MIA_USE; 
using namespace gsl; 

const SProgramDescrption g_general_help = {
	"Little helper", 
	"This program runs the wavelet based slope-statistics on the mixing matrix \n"
	"of a ICA perfusion series.", 
	NULL, 
	NULL
}; 

const TDictMap<EWaveletType>::Table wavelet_dict[] = {
	{"haar", wt_haar, "HAAR wavelet"},
	{"c-haar", wt_haar_centered, "HAAR wavelet (centered)"},
	{"daubechies", wt_daubechies, "Daubechies wavelet"},
	{"c-daubechies", wt_daubechies_centered, "Daubechies wavelet (centered)"},
	{"bspline", wt_bspline, "B-Spline wavelet"},
	{"c-bspline", wt_bspline_centered, "B-Spline wavelet (centered)"},
	{NULL, wt_none, ""}
};

const TDictMap<EWaveletType> g_wavelet_dict(wavelet_dict);


int do_main( int argc, const char *argv[] )
{
	CCmdOptionList options(g_general_help);

	string in_filename;
	string out_filename;


	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input data set", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 
	
	
	// read first line to estimate number of columns
	ifstream input(in_filename.c_str()); 
	char s[1024]; 
	input.getline(s, 1024); 
	istringstream first_row(s); 
	cvdebug() << "First row is '" << s << "'\n"; 
	vector<float> vx; 
	while (first_row.good()) {
		float x;
		first_row >> x; 
		if (!first_row.fail()) 
			vx.push_back(x); 
	}
	if ( vx.empty() ) 
		throw runtime_error("Empty input file"); 

	CWaveletSlopeClassifier::Columns table(vx.size());
	for (size_t i = 0; i < vx.size(); ++i) 
		table[i].push_back(vx[i]); 

	cvdebug() << "Read " << vx.size() << " columns\n"; 	
	// read the reminder of the table 
	while (input.good()) {
		for (size_t i = 0; i < vx.size() && input.good(); ++i) {
			float x; 
			input >> x; 
			if (input.good()) 
				table[i].push_back(x);
		}
	}

	size_t nrows = table[0].size(); 
	cvdebug() << "got " << nrows << "  rows\n"; 

	for (size_t i = 1; i < vx.size(); ++i) 
		if (table[i].size() != nrows) {
			THROW(runtime_error, "bogus input table, expect " << nrows  << ", but column " 
			      << i << " has " << table[i].size() << " rows"); 
		}

	CWaveletSlopeClassifier classifier(table, false);
	
	switch (classifier.result()) {
	case CWaveletSlopeClassifier::wsc_fail: 
		throw invalid_argument("The input data could not be classified into LV, RV and additional components"); 
	case CWaveletSlopeClassifier::wsc_no_movement: 
		cout << "The input data set doesn't contain significant motion\n"; 
		cout << "RV: idx = "<< classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n"; 
		cout << "LV: idx = "<< classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n"; 
		cout << "Baseline: "<< classifier.get_baseline_idx() << "\n";
		cout << "Perfusion:"<< classifier.get_perfusion_idx() << " (uncertain) \n";
		break; 
	case CWaveletSlopeClassifier::wsc_low_movement: 
		cout << "The input data set contains some motion\n"; 
		cout << "RV: idx = "<< classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n"; 
		cout << "LV: idx = "<< classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n"; 
		cout << "Baseline: "<< classifier.get_baseline_idx() << "\n";
		cout << "Motion:   "<< classifier.get_movement_idx() << " with energy " << classifier.get_movement_indicator() << "\n";
		cout << "Perfusion:"<< classifier.get_perfusion_idx() << " (uncertain) \n";
		break; 
	case CWaveletSlopeClassifier::wsc_normal: 
		cout << "The input data set contains motion\n";
		cout << "RV: idx = "<< classifier.get_RV_idx() << " with peak at " << classifier.get_RV_peak() << "\n"; 
		cout << "LV: idx = "<< classifier.get_LV_idx() << " with peak at " << classifier.get_LV_peak() << "\n"; 
		cout << "Baseline: "<< classifier.get_baseline_idx() << "\n";
		cout << "Motion:   "<< classifier.get_movement_idx() << " with energy " << classifier.get_movement_indicator() << "\n";
		cout << "Perfusion:"<< classifier.get_perfusion_idx() << " (uncertain) \n";
		break; 

	default: 
		cout << "hu?\n";
	}
	return EXIT_SUCCESS; 
}

int main( int argc, const char *argv[] )
{


	try {
		return do_main(argc, argv);
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}

		
