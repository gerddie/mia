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

/*
\remark The programm was origionall written by D.L. Pham and J.L.Prince, Pat. Rec. Let., 20:57-68,1999.
\remark The CG optimization was included by Stefan Burckhardt und Carsten Walters
\author Heike J"anicke and M. Tittgemeyer (tittge@cbs.mpg.de), 2004
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <mia/3d/fuzzyseg.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Fuzzy c-means segmentation of a 3D image."}, 
	{pdi_description, "This program runs a combined fuzzy c-means clustering and B-field correction "
	"to facilitate a 3D segmentation of 3D image"}, 
	{pdi_example_descr, "Run a 5-class segmentation over inpt image input.v and store the class "
	"probability images in cls.v and the B0-field corrected image in b0.v."}, 
	{pdi_example_code, "-i input.v -c 5 -o b0.v -c cls.v"}
}; 


int do_main( int argc, char *argv[] )
{


	string in_filename;
	string out_filename;
	string cls_filename;
	int    noOfClasses = 3;
	float  residuum = 0.1;

	const auto& imageio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be segmenetd", 
			      CCmdOptionFlags::required_input, &imageio));
	
	options.add(make_opt( cls_filename, "cls-file", 'c', "output class probability images. Note, the "
			      "used file format must support multible images (best is to use vista)", 
			      CCmdOptionFlags::output, &imageio));
	options.add(make_opt( out_filename, "b0-file", 'o', "image corrected for intensity non-uniformity", 
			      CCmdOptionFlags::output , &imageio));
	options.add(make_opt( noOfClasses, "no-of-classes", 'n', "number of classes"));
	options.add(make_opt( residuum, "residuum", 'r', "relative residuum"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	if (cls_filename.empty() && out_filename.empty())
		throw runtime_error("Not a single output file given"); 


	C3DImageIOPluginHandler::Instance::PData inImage_list = imageio.load(in_filename);

	if (!inImage_list.get() || !inImage_list->size() ) {
		string not_found = ("No supported data found in ") + in_filename;
		throw runtime_error(not_found);
	}

	// segment image
	if (inImage_list->size() > 1)
		cvwarn() << "Only segmenting first input image\n";

	C3DImageVector classes;

	P3DImage b0_corrected = fuzzy_segment_3d(**inImage_list->begin(), noOfClasses, residuum, classes);

	if (!out_filename.empty()) {

		// save corrected image to out-file
		C3DImageIOPluginHandler::Instance::Data out_list;

		out_list.push_back(b0_corrected);
		if ( !imageio.save(out_filename, out_list) ){

			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);

		};

	};

	//CHistory::instance().append(argv[0], revision, opts);

	if ( !imageio.save(cls_filename, classes) ){
		string not_save = ("unable to save result to ") + cls_filename;
		throw runtime_error(not_save);

	}


	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
