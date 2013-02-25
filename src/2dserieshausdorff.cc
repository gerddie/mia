/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cmath>
#include <list>
#include <cassert>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>

#include <mia/core.hh>
#include <mia/2d/SegSet.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>

using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;

const SProgramDescription g_description = {
        {pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Evaluate Hausdorff distance between segmentation sets."}, 
	{pdi_description, "Get the per-slice Hausdorff distance of a segmentation with "
	 "respect to a given reference segmentation set."}, 
	{pdi_example_descr, "Evaluate the per-frame Hausdorff distance of the segmentations "
	 "of set segment.set with respect to the segmentation set reference.set skipping "
	 "two images at the beginning."}, 
	{pdi_example_code, " -i segment.set -r reference.set -k 2"}
}; 

int do_main(int argc, char *argv[])
{
	string src_filename;
	string ref_filename;
	int skip = 0; 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( ref_filename, "ref-file", 'r', "reference segmentation set", CCmdOption::required));
	options.add(make_opt( skip, "skip", 'k', "skip images at the beginning"));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(src_filename);

	DomParser parser2;
	parser2.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser2.parse_file(ref_filename);

	if (!parser)
		throw runtime_error(string("Unable to parse input file:") + src_filename);

	if (!parser2)
		throw runtime_error(string("Unable to parse input file:") + ref_filename);

	CSegSet srcset(*parser.get_document());
	CSegSet refset(*parser2.get_document());

	const auto& src_frames = srcset.get_frames();
	const auto& ref_frames = refset.get_frames();

	if (src_frames.size() != ref_frames.size())
		throw invalid_argument("Input data sets must have the same number of slices"); 
	
	if (skip >= (int)src_frames.size()) 
		throw invalid_argument("Can't skip the whole series"); 

	auto isrc_frame = srcset.get_frames().begin() + skip;
	auto iref_frame = refset.get_frames().begin() + skip;
	auto esrc_frame = srcset.get_frames().end();

	while (isrc_frame != esrc_frame) {
		cout << iref_frame->get_hausdorff_distance(*isrc_frame) << "\n";
		++iref_frame;
		++isrc_frame; 
	}
	return 0;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
