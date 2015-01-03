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

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#include <mia/3d/imageio.hh>
#include <mia/2d/imageio.hh>
#include <mia/core.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Print the dimensions of a 3D image."}, 
	{pdi_description, "Write the dimensions of the input 3D image to stdout."}, 
	{pdi_example_descr, "Print out the size of image.v."}, 
	{pdi_example_code, "-i image.v"}
}; 

int do_main( int argc, char *argv[] )
{
	string in_filename;
	const auto& imageio3d = C3DImageIOPluginHandler::instance();
	
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &imageio3d));
	options.set_stdout_is_result();
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio3d.load(in_filename);
	
	
	if (in_image_list.get() && in_image_list->size()) {
		cout << (*in_image_list->begin())->get_size() << "\n";
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
