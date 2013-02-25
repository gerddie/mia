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
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/imageio.hh>
#include <mia/2d/imageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	{pdi_group, "Miscellaneous programs"}, 
	
	{pdi_short, "Add attributes to the meta data of a 3D image."}, 
	
	{pdi_description, "This program appends all the meta-data attributes found in a 2D input image "
	"to a 3D image (if the output format supports such attributes). "
	 "Attributes that are already present in the 3D image are not overwritten."}, 
	
	{pdi_example_descr, "Append the attributes found in 2d.v to 3d.v and store the result in attr3d.v"}, 

	{pdi_example_code, " -i 3d.v -o attr3d.v -a 2d.v"}
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	string attr_image;

	const auto& image2dio = C2DImageIOPluginHandler::instance();
	const auto& imageio = C3DImageIOPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output image(s) with the added attributes", 
			      CCmdOption::required, &imageio));
	options.add(make_opt( attr_image, "attr", 'a', "2D image providing the attributes", 
			      CCmdOption::required, &image2dio));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";

	//CHistory::instance().append(argv[0], "unknown", options);

	// read image
	auto  in_image_list = imageio.load(in_filename);
	auto  attr_image_list = image2dio.load(attr_image);


	if ( in_image_list.get() && in_image_list->size() &&
	     attr_image_list.get() && attr_image_list->size() ) {

		P2DImage attr_image = *attr_image_list->begin();
		for (auto i = in_image_list->begin();  i != in_image_list->end(); ++i)
			for (auto a = attr_image->begin_attributes();  a != attr_image->end_attributes(); ++a) {
				if (!(*i)->has_attribute(a->first))
					(*i)->set_attribute(a->first, a->second);
			}
		
	}
	if ( !imageio.save(out_filename, *in_image_list) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	}

	return EXIT_SUCCESS;
	
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
