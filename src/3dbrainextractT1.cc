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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <boost/filesystem/convenience.hpp>


#include <mia/3d/fuzzyseg.hh>
#include <mia/3d/filter.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/datapool.hh>

NS_MIA_USE
using namespace std;
namespace bfs = ::boost::filesystem;

const SProgramDescription g_description = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 

	{pdi_short, "Extract the brain from a 3D T1 MRI head image."}, 

	{pdi_description, "This program is used to extract the brain from T1 MR images. "
	"It first runs a combined fuzzy c-means clustering and B-field correction "
	"to facilitate a 3D segmentation of 3D image. "
	"Then various fiters are run to obtain a white matter segmentation as initial "
	"mask that is then used to run a region growing to obtain a mask of the whole brain. "
	 "Finally, this mask is used to extact the brain from the B0 field corrected images."}, 
	
	{pdi_example_descr, "Create a mask from the input image by running a 5-class segmentation over inpt image input.v "
	 "and use class 4 as white matter class and store the masked image in masked.v "
	 "and the B0-field corrected image in b0.v"}, 

	{pdi_example_code, "-i input.v -n 5 -w 4 -o masked.v"}
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	int    noOfClasses = 3;
	int    wmclass = noOfClasses - 1;
	float  residuum = 0.1;
	int    growthresh = 20;
	string growshape("18n");
	float  wmclassprob = 0.7;

	const auto& imageio = C3DImageIOPluginHandler::instance(); 
	
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
			      "input image(s) to be segmented", CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "brain mask", CCmdOptionFlags::required_output, &imageio));
	options.add(make_opt( noOfClasses, "no-of-classes", 'n', "number of classes"));
	options.add(make_opt( wmclass,     "wm-class",      'w', "index of white matter"));
	options.add(make_opt( wmclassprob, "wm-prob", 'p',
			      "white matter class probability for initial mask creation"));
	options.add(make_opt( growthresh, "grow-threshold", 't', "intensity threshold for region growing"));
	options.add(make_opt( growshape, "grow-shape", 0, "neighbourhood mask region growing"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	if (wmclass >= noOfClasses)
		throw invalid_argument("number of classes should be smaller then number of classes");

	auto inImage = load_image3d(in_filename);
	if (!inImage) {
		string not_found = ("No supported data found in ") + in_filename;
		throw runtime_error(not_found);
	}

	C3DImageVector classes;

	P3DImage b0_corrected = fuzzy_segment_3d(*inImage, noOfClasses, residuum, classes);
	
	const char *b0poolkey = "b0.@";
	// send image to data pool for later re-use. 
	save_image(b0poolkey, b0_corrected); 

	P3DImage result = classes[wmclass];

	// create filter chain
	vector<string> filter_chain;
	stringstream binarize;
	binarize << "binarize:min=" << wmclassprob;
	filter_chain.push_back(binarize.str());

	filter_chain.push_back("erode:shape=6n");
	filter_chain.push_back("label");
	filter_chain.push_back("selectbig");
	stringstream grow;
	grow << "growmask:ref="<< b0poolkey <<",min=" << growthresh << ",shape=" << growshape;
	filter_chain.push_back(grow.str());
	filter_chain.push_back("close:shape=[sphere:r=3]");
	filter_chain.push_back("open:shape=[sphere:r=3]");
	filter_chain.push_back(string("mask:input=") + b0poolkey);


	vector<P3DFilter> filters = create_filter_chain(filter_chain);

	for (auto f = filters.begin(); f != filters.end(); ++f) {
		result = (*f)->filter(*result);
	}


	if (!out_filename.empty()) {
		if ( !save_image(out_filename, result) ){
			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);

		};
	};

	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
