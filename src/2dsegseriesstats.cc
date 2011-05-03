/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#define VSTREAM_DOMAIN "2dmyosegstats"

#include <libxml++/libxml++.h>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <ostream>
#include <fstream>

using xmlpp::DomParser;

using namespace mia; 
using namespace std; 


const char *g_description = 
	"This program is used to evaluate average intensities over given\n"
	"segmented regions of an image"; 


struct SResult {
	float original; 
	float registered; 
	float hand; 
}; 

ostream& operator << (ostream& os, const SResult& r) 
{
	os << r.original << " " << r.registered << " " << r.hand; 
	return os; 
}


bool normalize_and_save_curves(vector<vector<SResult> >& curves, const string& curves_filename)
{
	// normalize 
	float max_hand = 0.0;
	float min_hand = numeric_limits<float>::max();
	for(auto i = curves.begin(); i != curves.end(); ++i)
		for(auto k = i->begin(); k != i->end(); ++k) {
			if (max_hand < k->hand) 
				max_hand = k->hand; 
			if (min_hand > k->hand)
				min_hand = k->hand; 
		}
	float div = 1.0/(max_hand - min_hand); 
	for(auto i = curves.begin(); i != curves.end(); ++i)
		for(auto k = i->begin(); k != i->end(); ++k) {
			k->hand = div * (k->hand - min_hand); 
			k->registered = div * (k->registered - min_hand); 
			k->original = div * (k->original - min_hand); 
		}
	
	// 
	ofstream outfile(curves_filename.c_str(), ios_base::out); 
	if (outfile.good())
		for(auto i = curves.begin(); i != curves.end(); ++i){
			for(auto k = i->begin(); k != i->end(); ++k)
				outfile << *k << " "; 
			outfile << "\n"; 
		}
	return outfile.good(); 
}

int do_main( int argc, const char *argv[] )
{
	string org_filename;
	string reg_filename;
	string curves_filename("curves.txt"); 
	string varcurves_filename("varcurves.txt"); 
	
	size_t n_sections = 0; 
	size_t skip = 2; 
	size_t reference = 20; 

	CCmdOptionList options(g_description);
	options.push_back(make_opt( org_filename, "original", 'o', "original segmentation set", CCmdOption::required));
	options.push_back(make_opt( reg_filename, "registered", 'g', "registered segmentation set", CCmdOption::required));
	options.push_back(make_opt( skip, "skip", 'k', "images to skip atthe bgin of the series")); 
	options.push_back(make_opt( reference, "reference", 'r', "reference image")); 
	options.push_back(make_opt( curves_filename, "curves", 'c', "region average value curves"));
	options.push_back(make_opt( varcurves_filename, "varcurves", 'v', "region variation values"));
	options.push_back(make_opt( n_sections, "nsections", 'n', 
				    "number of sections to use, 0=use as segmented")); 
	
	options.parse(argc, argv);

	CSegSetWithImages original(org_filename, true); 
	CSegSetWithImages registered(reg_filename, true); 

	auto original_frames = original.get_frames(); 
	auto registered_frames = registered.get_frames(); 
	
	
	if (original_frames.size() != registered_frames.size()) 
		THROW(invalid_argument, "original and reference series must have same size"); 
	if (reference < skip || reference >= original_frames.size())
		THROW(invalid_argument, "reference frame must be larger then skip="<<
		      skip << " and smaller then the length of the series " << original_frames.size()); 
	
	vector<vector<SResult> > curves; 
	vector<vector<SResult> > varcurves; 
	
	C2DUBImage org_mask = original_frames[reference].get_section_masks(n_sections); 
	C2DUBImage reg_mask = registered_frames[reference].get_section_masks(n_sections); 
	for (size_t i = skip; i < original_frames.size(); ++i)  {
		auto stats_unregistered  = original_frames[i].get_stats(org_mask);
		auto stats_registered    = registered_frames[i].get_stats(reg_mask);
		auto stats_handsegmented = original_frames[i].get_stats(n_sections); 

		if (stats_unregistered.size() != stats_registered.size() ||
		    stats_registered.size() != stats_handsegmented.size()) {
			THROW(runtime_error, "Frame " << i << " is not properly segmented,"
			      << " got org:" << stats_unregistered.size() 
			      << " reg:" << stats_registered.size() 
			      << " hand:" << stats_handsegmented.size()
			      );  
		}
			
		vector<SResult> c_row(stats_unregistered.size()); 
		vector<SResult> v_row(stats_unregistered.size()); 
		
		for (size_t k = 0; k< stats_unregistered.size(); ++k) {
			c_row[k].registered = stats_registered[k].first; 
			c_row[k].original = stats_unregistered[k].first; 
			c_row[k].hand = stats_handsegmented[k].first; 
			
			v_row[k].registered = stats_registered[k].second; 
			v_row[k].original = stats_unregistered[k].second; 
			v_row[k].hand = stats_handsegmented[k].second; 
		}
		curves.push_back(c_row); 
		varcurves.push_back(v_row);
	}

	
	if (!curves_filename.empty()) 
		if (!normalize_and_save_curves(curves, curves_filename)) 
			THROW(runtime_error, "Unable to write '" << curves_filename << "'"); 

	if (!varcurves_filename.empty()) 
		if (!normalize_and_save_curves(varcurves, varcurves_filename)) 
			THROW(runtime_error, "Unable to write '" << varcurves_filename << "'"); 
	
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
