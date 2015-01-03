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
#include <mia/2d/segset.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;


const SProgramDescription g_description = {
        {pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Evaluate the Hausdorff distance between segmentations."}, 
	{pdi_description, "This program is used to evaluate the Hausdorff distance between each frame "
	 "of a perfusion time series of the input set to the corresponding frame of the reference set "
	 "and prints the result to stdout."}, 
	{pdi_example_descr, "Evaluate the per-slice Hausdorff distance of input.set and reference.set."}, 
	{pdi_example_code, "-i input.set -r reference.set"}
}; 

CSegSet load_segmentation(const string& s)
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(s);
	return CSegSet(*parser.get_document());
}

int do_main(int argc, char *argv[])
{
	string src_filename;
	string ref_filename;

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( ref_filename, "ref-file", 'r', "reference  segmentation set", CCmdOptionFlags::required_input));
	options.set_stdout_is_result();
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset = load_segmentation(src_filename);
	CSegSet ref_segset = load_segmentation(ref_filename);

	const CSegSet::Frames& src_frames = src_segset.get_frames();
	const CSegSet::Frames& ref_frames = ref_segset.get_frames();

	if (ref_frames.size() != src_frames.size())
		throw invalid_argument("segmentations have different frame numbers");


	auto iframe = src_frames.begin();
	auto rframe = ref_frames.begin();
	auto eframe = src_frames.end();

	while (iframe != eframe) {
		const auto& src_sections = iframe->get_sections();
		const auto& ref_sections = rframe->get_sections();

		cout << iframe->get_hausdorff_distance(*rframe) << " ";
		if (src_sections.size() == ref_sections.size() ) 
			for (size_t j = 0; j < src_sections.size(); ++j)
				cout << src_sections[j].get_hausdorff_distance(ref_sections[j]) << " ";
		else 
			cout << "slice segmented with different number of sections"; 
		cout << "\n";

		++iframe;
		++rframe;
	}
	return 0;

}

MIA_MAIN(do_main); 
