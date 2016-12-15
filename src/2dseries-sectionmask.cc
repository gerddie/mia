/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
	{pdi_short,"Evaluate masks from a segmentation."}, 
	{pdi_description, "Evaluate the masks for the sections of a segmented frame."}, 
	{pdi_example_descr, "Evaluate the mask image from the segmentation of image 20 in segment.set "
	 "and store it to mask20.png:"},
	{pdi_example_code, "-i segment.set -f 20 -o mask20.png"}
}; 

int do_main(int argc, char *argv[])
{
	string src_filename;
	string out_filename;
	size_t frame = 0; 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', "output image containing the mask", 
			      CCmdOptionFlags::required_output, &C2DImageIOPluginHandler::instance()));
	options.add(make_opt( frame, "frame", 'f', "Frame number for which to extract the mask"));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset(src_filename);

	const CSegSet::Frames& src_frames = src_segset.get_frames();
	if (frame >= src_frames.size())
		throw create_exception<invalid_argument>("Requested frame ", frame, " out of range (", src_frames.size(), ")");

	auto mask = src_frames[frame].get_section_masks(); 
	if (!save_image(out_filename, mask)) 
		throw create_exception<runtime_error>( "Unable to save mask image to '", out_filename, "'"); 
	
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 
