/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#define VSTREAM_DOMAIN "2dmyoseries-dice"

#include <libxml++/libxml++.h>
#include <mia/core/msgstream.hh>
#include <mia/internal/main.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/segsetwithimages.hh>
#include <ostream>
#include <fstream>

using xmlpp::DomParser;

using namespace mia; 
using namespace std; 

const SProgramDescription g_description = {
        {pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Evaluate the dice index between segmentations."}, 
	{pdi_description, "This program is used to evaluate the per-frame dice index of "
	 "segmented regions of two image series"}, 
	{pdi_example_descr, "Evaluate the per-frame dice index of the segmentations of set segment.set with "
	 "respect to the segmentation set reference.set skipping two images at the beginning."}, 
	{pdi_example_code, "-i segment.set -r reference.set -k 2"}
}; 

float dice_value(const C2DUBImage& mask1, const C2DUBImage& mask2) 
{
	assert (mask1.get_size() == mask2.get_size()); 
	int schnitt = 0; 
	int sum = 0; 
	
	for (auto m1 = mask1.begin(), m2 = mask2.begin(); 
	     m1 != mask1.end(); ++m1, ++m2) {
	
		if (*m1) 
			++sum; 
		if (*m2) 
			++sum; 
		
		if (*m1 && *m2)
			++schnitt; 
	}
	
	return sum ? (2.0f * schnitt) / sum : 1.0f; 
}

int do_main( int argc, char *argv[] )
{
	string org_filename;
	string ref_filename;
	size_t skip = 2; 

	CCmdOptionList options(g_description);
	options.add(make_opt( org_filename, "first", '1', "first segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( ref_filename, "second", '2', "second segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( skip, "skip", 'k', "images to skip atthe begin of the series")); 
	options.set_stdout_is_result();
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSetWithImages original(org_filename, true); 
	CSegSetWithImages ref(ref_filename, true); 

	auto original_frames = original.get_frames(); 
	auto reference_frames = ref.get_frames(); 

	if (original_frames.size() != original_frames.size())
		throw invalid_argument("Both series must contain the same number of frames"); 
	
	if (skip  >= original_frames.size())
		throw create_exception<invalid_argument>( "skip (", skip, ") must be smaller then number of availabe frames(", 
						original_frames.size(), ")"); 
	


	for (size_t i = skip; i < original_frames.size(); ++i)  {
		auto o = original_frames[i].get_section_masks(1);
		auto r = reference_frames[i].get_section_masks(1);
		cout << dice_value(o, r) << '\n'; 
	}
	
	return EXIT_SUCCESS; 
}

MIA_MAIN(do_main); 
