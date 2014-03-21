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

#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;


const SProgramDescription g_description = {
        {pdi_group, "Image conversion"}, 
	{pdi_short, "Select one image from a multi-record file."}, 
	{pdi_description, "This program is used to select one 2D images from a multi-image file."}, 
	{pdi_example_descr, "Store the third image in multiimage.v to image.v (note: counting starts with zero)."}, 
	{pdi_example_code, "-i multiimage.v -o image.v -n 2"}
}; 

int do_main( int argc, char *argv[])
{

	string in_filename;
	string out_filename;
	size_t num = 0;

	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
				    "input images", CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image", CCmdOptionFlags::required_output, &imageio));
	options.add(make_opt( num, "number", 'n',  "image number to be selected"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read image
	C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	if (!in_image_list) {
		stringstream msg;
		msg << "No images found in '" << in_filename <<"'";
		throw invalid_argument(msg.str());
	}

	if (!( num < in_image_list->size() )) {
		stringstream msg;
		msg << "Request image nr. " << num << ", but input file '"<< in_filename
		    <<"'has only " << in_image_list->size() << " images";
		throw invalid_argument(msg.str());
	}

	C2DImageIOPluginHandler::Instance::Data out_image_list;
	out_image_list.push_back( (*in_image_list)[num]);

	if ( !imageio.save(out_filename, out_image_list) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};

	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
