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

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	"3D image processing", 
	 
	"This program is used to combine two images using a given image combiner plug-in (combiner/3dimage).", 
	
	"Take two label images l1.v and l2.v and evaluate the label overlap.", 

	"-1 l1.v -2 l2.v -c map.txt -c labelxmap" 
};  

int do_main( int argc, char *argv[] )
{

	string in_image1;
	string in_image2;
	string out_filename;
	string combiner_descr;

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;

	CCmdOptionList options(g_description);
	options.add(make_opt( in_image1, "image1", '1', "input image  1 to be combined", CCmdOption::required));
	options.add(make_opt( in_image2, "image2", '2', "input image  2 to be combined", CCmdOption::required));
	options.add(make_opt( combiner_descr, "combiner", 'c', "combiner operation", CCmdOption::required));
	options.add(make_opt( out_filename, "out", 'o', "output file", CCmdOption::required));

	options.add(make_help_opt( "help-plugins", 0,
					 "give some help about the filter plugins", 
					 new TPluginHandlerHelpCallback<C3DImageCombinerPluginHandler>)); 
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

	P3DImageCombiner combiner =  C3DImageCombinerPluginHandler::instance().produce(combiner_descr.c_str());

	PCombinerResult combination = combiner->combine(**image1list->begin(), **image2list->begin());

	combination->save(out_filename);

	return 0;
};



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
