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


#define VSTREAM_DOMAIN "3dmyoserial"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/filetools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescription g_general_help = {
	"Registration of series of 3D images", 
	
	"This program runs the image registration of a consecutively numbered image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession (i.e. consecutive numbers) are registered, and the obtained transformations "
	"are applied accumulated to reach full registration. ", 
	
	"Run a serial registration of images inputXXXX.v (X digit) to reference image 20 and store the result in regXXXX.v. "
	"Optimize the sum of squared differences and spline transformations with coefficient rate 10.", 

	"-i input0000.v -o 'reg%04d.v' -f spline:rate=10 -r 20 ssd"
}; 

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg%04d.v");
	                        
	P3DTransformationFactory transform_creator; 

	// registration parameters
	PMinimizer minimizer;
	size_t mg_levels = 3; 
	int reference_param = -1; 
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', "file name for registered fiels")); 
	
	
	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "spline", "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 

	if (options.parse(argc, argv, "cost", &C3DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	C3DFullCostList costs; 
	for (auto c = cost_functions.begin(); c != cost_functions.end(); ++c) {
		auto cost = C3DFullCostPluginHandler::instance().produce(*c); 
		costs.push(cost); 
	}
	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	C3DImageSeries input_images; 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images.push_back(image);
	}

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images.size() / 2 : reference_param; 

	// prepare registration framework 
	C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images.size() - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	
	// run forward registrations 
	for (size_t i = 0; i < reference; ++i) {
		P3DTransformation transform = nrr.run(input_images[i], input_images[i+1]);
		for (size_t j = 0; j <=i ; ++j) {
			input_images[j] = (*transform)(*input_images[j]);
		}
	}
	
	// run backward registration 
	for (size_t i = input_images.size() - 1; i > reference; --i) {
		P3DTransformation transform = nrr.run(input_images[i], input_images[i-1]);
		for (size_t j = input_images.size() - 1; j >= i ; --j) {
			input_images[j] = (*transform)(*input_images[j]);
		}
	}
	

	bool success = true; 
	auto ii = input_images.begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
		cvmsg() << "Save image " << i << " to " << out_name << "\n"; 
		success &= save_image(out_name, *ii); 
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main); 
