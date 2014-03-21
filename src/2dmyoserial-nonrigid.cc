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

#define VSTREAM_DOMAIN "2dmyoserial"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 


const SProgramDescription g_general_help = {
        {pdi_group, "Registration of series of 2D images"}, 
	{pdi_short, "Run a serial registration of a series of 2D images."}, 
	{pdi_description, "This program runs the non-rigid motion compensation registration of an perfusion image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession are registered, and the obtained transformations "
	"are applied accumulated to reach full registration. "
	"See e.g. Wollny, G., Ledesma-Carbayo, M.J., Kellman, P., Santos, A. \"A New Similarity Measure "
	"for Non-Rigid Breathing Motion Compensation of Myocardial Perfusion MRI \". Proc 30th Annual "
	 "International IEEE EMBS Conference, pp. 3389-3392. Vancouver, Aug. 2008, doi:10.1109/IEMBS.2008.4649933,"}, 
	
	{pdi_example_descr, "Register the perfusion series given in 'segment.set' to reference image 30. " 
        "Skip two images at the beginning and using mutual information as cost function, "
	"and penalize the transformation by divcurl with weight 5. "
	 "Store the result in 'registered.set'.\n"}, 
	
	
	{pdi_example_code, "  -i segment.set -o registered.set -k 2  -r 30 image:cost=mi -f spline:rate=5,penalty=[divcurl:weight=5]"}
}; 

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");
	                        
	P2DTransformationFactory transform_creator; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// registration parameters
	PMinimizer minimizer;
	size_t mg_levels = 3; 
	int reference_param = -1; 
	int skip = 0; 
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
			      "input perfusion data set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', 
			      "output perfusion data set", CCmdOptionFlags::required_output));
	options.add(make_opt( registered_filebase, "registered", 'R', 
				    "file name base for registered fiels")); 
	
	
	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "spline:rate=16,penalty=[divcurl:weight=0.01]", "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 
	options.add(make_opt( skip, "skip", 'k', "skip registration of these images at the beginning of the series")); 

	if (options.parse(argc, argv, "cost", &C2DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	C2DFullCostList costs; 
	for (auto c = cost_functions.begin(); c != cost_functions.end(); ++c) {
		auto cost = C2DFullCostPluginHandler::instance().produce(*c); 
		costs.push(cost); 
	}

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images.size() / 2 : reference_param; 


	// prepare registration framework 
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images.size() - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	
	// run forward registrations 
	for (size_t i = skip; i < reference; ++i) {
		P2DTransformation transform = nrr.run(input_images[i], input_images[i+1]);
		for (size_t j = 0; j <=i ; ++j) {
			input_images[j] = (*transform)(*input_images[j]);
			frames[j].inv_transform(*transform); 
		}
	}
	
	// run backward registration 
	for (size_t i = input_images.size() - 1; i > reference; --i) {
		P2DTransformation transform = nrr.run(input_images[i], input_images[i-1]);
		for (size_t j = input_images.size() - 1; j >= i ; --j) {
			input_images[j] = (*transform)(*input_images[j]);
			frames[j].inv_transform(*transform); 
		}
	}
	
	
	// prepare output set and save images 
	input_set.set_images(input_images); 
	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	input_set.set_preferred_reference(reference); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

}





#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

