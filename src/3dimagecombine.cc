/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images" }, 
	{pdi_short, "Combine two 3D images."}, 
	{pdi_description, "This program is used to combine two images using a given image combiner."}, 
	{pdi_example_descr, "Take two images l1.v and l2.v and evaluate the per-voxel sum."}, 
	{pdi_example_code, "-1 l1.v -2 l2.v -c sum.v -c add"}
};  


int do_main( int argc, char *argv[] )
{

	string in_image1;
	string in_image2;
	string out_filename;
	P3DImageCombiner combiner;

	const auto& imageio = C3DImageIOPluginHandler::instance();
	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;

	CCmdOptionList options(g_description);
	options.add(make_opt( in_image1, "image1", '1', "input image  1 to be combined", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( in_image2, "image2", '2', "input image  2 to be combined", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( combiner, "add", "combiner", 'c', "combiner operation", CCmdOptionFlags::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output image file", CCmdOptionFlags::required_output, &imageio));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	cvdebug() << "Load images from '" << in_image1 << "' and '" << in_image2 << "'\n";
	PImageVector image1list = imageio.load(in_image1);
	PImageVector image2list = imageio.load(in_image2);


	if (!image1list || image1list->size() < 1) {
		cverr() << "no image found in " << in_image1 << "\n";
		return EXIT_FAILURE;
	}

	if (!image2list || image2list->size() < 1) {
		cverr()  << "no image found in " << in_image2 << "\n";
		return EXIT_FAILURE;
	}

	if (image1list->size() > 1)
		cvwarn() << "only first image in " << in_image1 << "will be used\n";

	if (image2list->size() > 1)
		cvwarn() << "only first image in " << in_image2 << "will be used\n";

	auto combination = combiner->combine(**image1list->begin(), **image2list->begin());

	if (save_image(out_filename, combination)) 
		return EXIT_SUCCESS; 

	return EXIT_FAILURE; 
};



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
