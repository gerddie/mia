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

#define VSTREAM_DOMAIN "2dmyosegstats"

#include <libxml++/libxml++.h>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/internal/main.hh>
#include <ostream>
#include <fstream>

using xmlpp::DomParser;

using namespace mia; 
using namespace std; 

const SProgramDescription g_description = {
	"Tools for Myocardial Perfusion Analysis", 

	"Evaluate time-intensity curves in masked regions of image series.", 	
	
	"This program is used evaluate various time-intensity curves over a series of images "
	"given by a segmentation set. Specifically, the program is taylored to evaluate average "
	"intensities and variations of sections the left ventricle myocardium. "
	"The segmentation set must contain the segmentations for all slices that will be accessed "
        "during evaluation. ",

	"Evaluate the two curve typed for 12 sections from segemntation sets orig.set "
	"and reg.set skipping the first 2 frames. The output will be written to curves.txt "
	"and varcurves.txt respectively.", 
	
	"-i org.set -g reg.set -c curves.txt -v varcurves.txt -n 12 -k 2"
}; 

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

int do_main( int argc, char *argv[] )
{
	string org_filename;
	string reg_filename;
	string curves_filename("curves.txt"); 
	string varcurves_filename("varcurves.txt"); 
	
	size_t n_sections = 0; 
	int skip = 2; 
	int reference = 20; 

	CCmdOptionList options(g_description);
	options.add(make_opt( org_filename, "original", 'o', "original segmentation set", CCmdOption::required));
	options.add(make_opt( reg_filename, "registered", 'g', "registered segmentation set", CCmdOption::required));
	options.add(make_opt( skip, "skip", 'k', "images to skip at the begin of the series, if (k < 0) use RV peak of the registered set if set")); 
	options.add(make_opt( reference, "reference", 'r', "reference image")); 
	options.add(make_opt( curves_filename, "curves", 'c', "region average value curves, "
			      "The output files each comprises a table in plain-text format that contains three columns "
			      "for each section of the LV myocardium: The first column contains the values obtained by "
			      "using the original segmentation of the reference on all images of the original series, "
			      "the second column containes the values obtained by the registered segmentation of the "
			      "reference on all images of the registered series, and the third column contains the "
			      "values obtained by using the segmentations of each slice on the original images."));
	options.add(make_opt( varcurves_filename, "varcurves", 'v', "region variation values, same formt as described above. "));
	options.add(make_opt( n_sections, "nsections", 'n', 
			      "number of sections to use, 0=use as segmented, otherwise Otherwise, the LV myocardium is "
			      "divided into n sections that enclose equal angles starting at the "
			      "right ventricle insertion point moving clock-wise using the LV center "
			      "as angular point.")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSetWithImages original(org_filename, true); 
	CSegSetWithImages registered(reg_filename, true); 

	if (skip < 0) {
                // if RV peak is given in the segmentation file, use it, otherwiese use 
		// absolue value of skip 
		int sk = registered.get_RV_peak(); 
		skip = (sk < 0 ) ? -skip : sk; 
	}

	auto original_frames = original.get_frames(); 
	auto registered_frames = registered.get_frames(); 
	
	
	if (original_frames.size() != registered_frames.size()) 
		throw create_exception<invalid_argument>( "original and reference series must have same size"); 
	if (reference < skip || reference >= static_cast<long>(original_frames.size()))
		throw create_exception<invalid_argument>( "reference frame must be larger then skip=", 
						skip, " and smaller then the length of the series ", original_frames.size()); 
	
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
			throw create_exception<runtime_error>( "Frame ", i, " is not properly segmented,", 
						     " got org:", stats_unregistered.size(), 
						     " reg:", stats_registered.size(), 
						     " hand:", stats_handsegmented.size());  
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
			throw create_exception<runtime_error>( "Unable to write '", curves_filename, "'"); 

	if (!varcurves_filename.empty()) 
		if (!normalize_and_save_curves(varcurves, varcurves_filename)) 
			throw create_exception<runtime_error>( "Unable to write '", varcurves_filename, "'"); 
	
	return EXIT_SUCCESS; 
}

MIA_MAIN(do_main); 
