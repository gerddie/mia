/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/core.hh>
#include <mia/2d/segset.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;
namespace bfs=boost::filesystem;

const SProgramDescription g_description = {
	{pdi_group, "Tools for Myocardial Perfusion Analysis"}, 

	{pdi_short, "Translate a segmentation by the given amount."}, 
	
	{pdi_description, "This program move the segmentation(s) of an image series by using a shift "
	 "that is given on a per-slice base. The program can be used to correct the "
	 "segmentation of the images if a linear registration was executed that only "
	 "applies a translation and does not correct the segmentation automatically. "}, 

	{pdi_example_descr, "Shirt the segmentation given in set segment.set by files shiftXXXX.txt and "
	 "store the result in shifted.set. Note, the numbering of the shiftXXXX.txx files must coincide."}, 
	
	{pdi_example_code, "-i segment.set -o shifted.set -g shifted -S shift"},  
}; 

static string get_number(const string& fname)
{
	bfs::path f(fname);
	string the_stem = f.stem().string();
	auto rs = the_stem.rbegin();
	string result;
	while (rs != the_stem.rend() && isdigit(*rs))
		result.insert(0,1,*rs++);
	return result;
}

int do_main(int argc, char *argv[])
{
	string src_filename;
	string out_filename;
	string shift_filename("crop");

	string shift_value_filebase("shift");

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', "output segmentation set", CCmdOptionFlags::required_output));
	options.add(make_opt( shift_filename, "image-file", 'g', "output image filename base"));

	options.add(make_opt( shift_value_filebase, "shift", 'S', "shift of segmentation - base name ", 
				    CCmdOptionFlags::required));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset(src_filename);
	CSegSet::Frames& frames = src_segset.get_frames();

	for (auto i = frames.begin(); i != frames.end(); ++i) {
		string nr = get_number(i->get_imagename());
		stringstream shift_file_name;
		shift_file_name << shift_value_filebase << nr << ".txt";
		ifstream shift_file(shift_file_name.str());
		C2DFVector shift;
		shift_file >> shift;
		i->shift(shift, i->get_imagename());
	}


	auto  outset = src_segset.write();

	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset.write_to_string();

	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main); 
