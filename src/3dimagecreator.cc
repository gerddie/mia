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


#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 

	{pdi_short, "Create a synthetic 3D image."}, 

	{pdi_description, "This program creates a 3D image that contains an object created by one of the object "  
	 "creator plug-ins (creator/3dimage)"},

	{pdi_example_descr, "Create an image output.v of size <64,128,256> that contains a lattic with "
	 "frequencys 8, 16, and 4 in x, y and, z-direction respectively."},

	{pdi_example_code, "-o lattic.v -j lattic:fx=8,fy=16,fz=4 -s \"<64,128,256>\""}
}; 
	
int do_main(int argc, char *argv[])
{
	C3DImageCreatorPluginHandler::ProductPtr object_creator;
	string out_filename;
	EPixelType pixel_type = it_ubyte;
	C3DBounds size(128,128,128);

	CCmdOptionList options(g_description);

	options.add(make_opt( out_filename, "out-file", 'o', "output file for create object", 
			      CCmdOptionFlags::required_output, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( size, "size", 's', "size of the object"));
	options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type "));
	options.add(make_opt( object_creator, "sphere", "object", 'j', "object to be created"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	P3DImage image = (*object_creator)(size, pixel_type);
	if (!image) {
		std::stringstream error;
		error << "Creator '" << object_creator->get_init_string() << "' could not create object of size " << size 
		      << " and type " << CPixelTypeDict.get_name(pixel_type);
		throw invalid_argument(error.str());
	}
	return !save_image(out_filename, image);
}



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
