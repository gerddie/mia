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

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/core.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/imagecollect.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

const SProgramDescription g_description = {
        {pdi_group, "Image conversion"}, 
	{pdi_short, "Combine a series of 2D images to a volume."}, 
	{pdi_description, "This program is used to convert a series 2D images into a 3D image. "
	 "The 2D images are read as additional command line parameters and the slice "
	 "ordering corresponds to the ordering of the file names on the commend line."}, 
	{pdi_example_descr, "Convert a series of images imageXXXX.png to a 3D image 3d.v"}, 
	{pdi_example_code, "-i imageXXXX.png -o 3d.v"}
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;

	CCmdOptionList options(g_description);
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", 
			      CCmdOption::required, &C3DImageIOPluginHandler::instance()));

	if (options.parse(argc, argv, "sliceimage", &C2DImageIOPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
		
	if (options.get_remaining().empty())
		throw runtime_error("no slices given ...");


	char new_line = cverb.show_debug() ? '\n' : '\r';

	auto input_images = options.get_remaining();

	C3DImageCollector ic(input_images.size());

	for (auto  i = input_images.begin(); i != input_images.end(); ++i) {

		cvmsg() << "Load " << *i << new_line;
		auto in_image = load_image2d(*i);
		ic.add(*in_image);
	}
	cvmsg() << "\n";

        if (save_image(out_filename, ic.get_result()))
		return EXIT_SUCCESS;
	else
		cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;
	return EXIT_FAILURE;
}
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
