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

using namespace std; 

NS_MIA_USE; 
using namespace gsl; 

const char *g_general_help = 
	"This program runs the wavelet based slope-statistics on the mixing matrix \n"
	"of a ICA perfusion series\n\n"
	"Basic usage: \n"
	" mia-wavelettrans [options] "; 

const TDictMap<EWaveletType>::Table wavelet_dict[] = {
	{"haar", wt_haar},
	{"c-haar", wt_haar_centered},
	{"daubechies", wt_daubechies},
	{"c-daubechies", wt_daubechies_centered},
	{"bspline", wt_bspline},
	{"c-bspline", wt_bspline_centered},
	{NULL, wt_none}
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

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
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

	vector<vector<float> > table(vx.size());
	for (int i = 0; i < vx.size(); ++i) 
		table[i].push_back(vx[i]); 

	cvdebug() << "Read " << vx.size() << " columns\n"; 	
	// read the reminder of the table 
	while (input.good()) {
		for (int i = 0; i < vx.size() && input.good(); ++i) {
			float x; 
			input >> x; 
			if (input.good()) 
				table[i].push_back(x);
		}
	}

	int nrows = table[0].size(); 
	cvdebug() << "got " << nrows << "  rows\n"; 

	for (int i = 1; i < vx.size(); ++i) 
		if (table[i].size() != nrows) {
			THROW(runtime_error, "bogus input table, expect " << nrows  << ", but column " 
			      << i << " has " << table[i].size() << " rows"); 
		}
	for (int i = 0; i < table.size(); ++i) {
		CSlopeStatistics stats(table[i]);
		auto idx = stats.get_peak_level_and_time_index(); 
		cout << "Mixing curve "<< i; 
		cout << "  mean frequency level= " << stats.get_mean_frequency_level() 
		     << "; energy= " << stats.get_wavelet_energy()
		     << "; max coefficient=" << stats.get_peak_wavelet_coefficient() 
		     << " at level=" << idx.first 
		     << " time idx=" << idx.second 
		     << "\n"; 
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

		
