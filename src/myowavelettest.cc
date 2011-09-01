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

	vector<CSlopeStatistics> vstats; 
	for (int i = 0; i < table.size(); ++i) {
		CSlopeStatistics stats(table[i]);
		auto idx = stats.get_peak_level_and_time_index(); 
		vstats.push_back(stats); 
	}

	/// classify 
	int movement_idx = vstats[0].get_level_coefficient_sums().size() - 2; 
	
	// first estimate if this is free breathing or breath holding 
	vector<float> movement_pos(4, 0); 
	for (size_t i = 0; i < table.size(); ++i)
		movement_pos[vstats[i].get_level_mean_energy_position()[movement_idx]] += vstats[i].get_level_coefficient_sums()[movement_idx];
	
	bool is_free_breathing = (movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_begin] && 
				  movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_end]); 
		
	cvmsg() << "Detected free breathing data set\n"; 
	int low_energy_start_idx = is_free_breathing ? 0 : 1; 
	
	// get movement components 
	vector<int> movement_indices; 
	vector<int> remaining_indices; 

	int min_energy_idx = -1; 
	float min_energy = numeric_limits<float>::max(); 

	for (size_t i = 0; i < table.size(); ++i) {
		auto e = vstats[i].get_level_coefficient_sums(); 
		cvdebug() << "energies: "  << e 
			  << " start= "<< low_energy_start_idx
			  << " end= " << movement_idx - 2 
			  << "\n"; 
		float low_freq = accumulate(e.begin() + low_energy_start_idx, e.begin() + movement_idx - 1, 0.0); 
		cvdebug() << i << ": low " << low_freq << " vs  high " << e[movement_idx] << "\n"; 

		if (min_energy > vstats[i].get_wavelet_energy()) {
			min_energy = vstats[i].get_wavelet_energy(); 
			min_energy_idx = i; 
		}
		
		if (low_freq < e[movement_idx]) 
			movement_indices.push_back(i); 
		else 
			remaining_indices.push_back(i); 
	}
	cvmsg() << "Baseline " << min_energy_idx 
		<<  " with energy " << min_energy 
		<< "\n"; 
	cvmsg() << "Detect movement candidates " << movement_indices << "\n"; 
	
	if (movement_indices.empty()) 
		throw runtime_error("no candidate component for movement\n"); 
	
	vector<float> movement_energies; 
	float max_movment_energy = 0.0; 
	int max_movment_idx = -1; 
	
	for (auto k = movement_indices.begin(); k != movement_indices.end(); ++k) {
		float energy = vstats[*k].get_level_coefficient_sums()[movement_idx]; 
		if (max_movment_energy < energy ) {
			max_movment_energy = energy; 
			max_movment_idx = *k;
		}
		if (*k == min_energy_idx) 
			continue; 
		movement_energies.push_back(energy);
	}
	if (max_movment_idx < 0) {
		cvmsg() << "No movement detected\n";  
		return EXIT_SUCCESS; 
	}	

	if (max_movment_idx == min_energy_idx) {
		cvmsg() << "Movement coincedes with the base line, time to stop\n"; 
		return EXIT_SUCCESS; 
	}

	for (auto k = remaining_indices.begin(); k != remaining_indices.end(); ++k) {
		if (vstats[*k].get_level_coefficient_sums()[movement_idx] > max_movment_energy) {
			cvmsg() << "Estimated maximum movement component " << max_movment_idx 
				<< " has less movement frequency energy than other component " << *k << " , time to stop?\n"; 
			return EXIT_SUCCESS; 
		}
	}
	
	cvmsg() << "max movement idx = " << max_movment_idx 
		<< " with movment energy " << max_movment_energy 
		<< " and full energy " <<  vstats[max_movment_idx].get_wavelet_energy()
		<< "\n"; 
	
	if (movement_energies.size() > 1) {
		sort(movement_energies.begin(), movement_energies.end(), [](float x, float y){return x > y;}); 
		cvmsg() << "movement_energys = " << movement_energies << "\n";
		
		cvmsg() << " Min Movement energy delta = " << movement_energies[0] - movement_energies[1] << "\n"; 
	}else{
		cvmsg() << "got exactly one movement candidate\n"; 
	}
	
	// classification of the remaining components 
	if (remaining_indices.size() < 2) 
		throw invalid_argument("too few components to separate LV and RV enhancement"); 

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

		
