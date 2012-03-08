/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#include <mia/3d/3dimageio.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	"Analysis, filtering, combining, and segmentation of 3D images", 
	
	"Write the dimensions of the input 3d image to stdout.", 
	
	"Print out the size of image.v.", 
	
	"-i image.v"
}; 

int do_main( int argc, char *argv[] )
{
	string in_filename;
	const C3DImageIOPluginHandler::Instance& imageio3d = C3DImageIOPluginHandler::instance();
	
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOption::required));
	
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
