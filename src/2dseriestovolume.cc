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
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/imageio.hh>
#include <mia/3d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/internal/main.hh>
#include <mia/3d/imagecollect.hh>

using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;


const SProgramDescription g_description = {
	{pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short,"Create a 3D volume from a segmentation series."}, 
	{pdi_description, "Obtaines a 3D volume image by combining the images of the segmentation set."}, 
	{pdi_example_descr, "Create the volume volume.v from the series series.set but skip the first two images."},
	{pdi_example_code, "-i series.set -k 2 -o volume.v"}
}; 


int do_main(int argc, char *argv[])
{
	string src_filename;
	string out_filename;
	size_t skip = 0; 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", 
			      CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output 3D image ", 
			      CCmdOption::required, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( skip, "skip", 'k', "number of frames to skip at the beginning of the series."));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSetWithImages src_segset(src_filename, false); 
	const auto& src_frames = src_segset.get_images();

	if (skip >= src_frames.size())
		throw create_exception<invalid_argument>("want to skip ", skip, 
							 " frames, but series only has ", src_frames.size(), " frames"); 

        C3DImageCollector ic(src_frames.size() - skip); 

	for (auto i = src_frames.begin() + skip; i != src_frames.end(); ++i)
		ic.add(**i); 
	
	if (save_image(out_filename, ic.get_result()))
		return EXIT_SUCCESS;
	else
		cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;
	return EXIT_FAILURE;

}

MIA_MAIN(do_main); 
