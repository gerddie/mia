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

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/core.hh>
#include <mia/2d/imageio.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/imagecollect.hh>


using namespace std;
NS_MIA_USE;


const SProgramDescription g_description = {
        {pdi_group, "Image conversion"}, 
	{pdi_short, "Convert a 2D multi-record image to a 3D image."}, 
	{pdi_description, "This program is used to convert a 2D multi-image file to a 3D image."}, 
	{pdi_example_descr, "Convert an series of images stored in a 2D image container multiimage2d.v to a 3D image image3d.v."}, 
	{pdi_example_code, "-i multiimage2d.v -o image3d.v"}
}; 

int do_main( int argc, char *argv[])
{

	string in_filename;
	string out_filename;

	const auto& imageio2d = C2DImageIOPluginHandler::instance();
	const auto& imageio3d = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
				    "input images", CCmdOptionFlags::required_input, &imageio2d));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image", CCmdOptionFlags::required_output, &imageio3d));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
	// read image
	C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio2d.load(in_filename);
	if (!in_image_list)
		throw create_exception<invalid_argument>("No images found in '", in_filename, "'");

	C3DImageCollector ic(in_image_list->size());

	for_each(in_image_list->begin(), in_image_list->end(), 
		 [&ic](P2DImage image){ ic.add(*image);}); 
	
	if (!save_image(out_filename, ic.get_result())) 
		throw create_exception<runtime_error>("Unable to write image to '", out_filename, "'");
	
	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
