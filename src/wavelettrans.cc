/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

using namespace std; 

NS_MIA_USE; 
using namespace gsl; 

const SProgramDescription g_general_help = {
	"Miscellaneous programs", 
	"This program runs the a column-wise discret wavelet forward transform on "
	"the data given by an input text file.", 
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


void save_wavelet(const string& filenamebase, const vector<double>& coeffs, int column)
{
	
	auto c = coeffs.begin() + 1; 
	int repeat = coeffs.size();
	int length = coeffs.size();
	int level = 0; 
	vector<vector<double> > help; 
	vector<double> index(length); 
	for (int i = 0; i < length; ++i) 
		index[i] = i; 
	while (repeat > 1)  {
		int i = 0; 
		vector<double> col(length); 
		while ( i < length ) {
			for ( int k = 0; k < repeat; ++k, ++i) 
				col[i] = fabs(*c); 
			++c; 
		}
		help.push_back(col); 
		repeat /=2; 
		++level; 
	}
	help.push_back(index); 
	stringstream fname; 
	fname << filenamebase << column << ".txt"; 
	ofstream file(fname.str().c_str()); 
	for (int i = 0; i < length; ++i) {
		for (int k = help.size() - 1; k >= 0; --k) {
			file << fabs(help[k][i]) << " "; 
		}
		file << '\n'; 
	}
	

}

int do_main( int argc, char *argv[] )
{
	CCmdOptionList options(g_general_help);

	string in_filename;
	string out_filename;
	string out2_filenamebase;

	EWaveletType wt_type = wt_daubechies_centered; 
	int k = 10; 


	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
			      "output data set", CCmdOption::required));

	options.add(make_opt(out2_filenamebase, "save-wave", 's', 
			     "base name of the output files that will store the wavelet coefficients as matrix")); 

	options.set_group("Wavelet"); 
	options.add(make_opt( wt_type, g_wavelet_dict, "wavelet", 'w', "wavelet to be used"));
	options.add(make_opt( k, "member", 'm', "wavelet member ID"));

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

	vector<vector<double> > table(vx.size());
	for (unsigned int i = 0; i < vx.size(); ++i) 
		table[i].push_back(vx[i]); 

	cvdebug() << "Read " << vx.size() << " columns\n"; 	
	// read the reminder of the table 
	while (input.good()) {
		for (unsigned int i = 0; i < vx.size() && input.good(); ++i) {
			float x; 
			input >> x; 
			if (input.good()) 
				table[i].push_back(x);
		}
	}

	int nrows = table[0].size(); 
	cvdebug() << "got " << nrows << "  rows\n"; 

	for (unsigned int i = 1; i < vx.size(); ++i) 
		if (table[i].size() != (unsigned)nrows) {
			THROW(runtime_error, "bogus input table, expect " << nrows  << ", but column " 
			      << i << " has " << table[i].size() << " rows"); 
		}
	
	// run the wavelet tranform for each column 
	C1DWavelet wt(wt_type, k); 
	for (unsigned int i = 0; i < vx.size(); ++i) {
		cvdebug() << "transfrom column of size " << table[i].size() <<"\n"; 
		table[i] = wt.forward(table[i]); 
		cverb << " result size = " << table[i].size() << "\n"; 
	}
	
	// save the result
	ofstream result(out_filename); 

	for (unsigned int r = 0; r < table[0].size(); ++r) {
		for (unsigned int c = 0; c < table.size(); ++c) {
			result << fabs(table[c][r]) << " "; 
		}
		result << "\n"; 
	}

	if (!out2_filenamebase.empty())  {
		// save the wavelet coefrficents as matrix 
		for (unsigned int c = 0; c < table.size(); ++c) {
			save_wavelet(out2_filenamebase, table[c], c); 
		}
		
	}
	
	return !result.good();

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
