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

#define VSTREAM_DOMAIN "2dmany3one"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/filetools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/imageio.hh>
#include <mia/internal/main.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

using namespace tbb;
using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 


const SProgramDescription g_general_help = {
        {pdi_group, "Registration of series of 2D images"}, 
	{pdi_short, "Registration of a series of 2D images"}, 
	{pdi_description, "This program registers all images of a conscutively numbered set of images to one common "
	 "user defined reference."}, 
	{pdi_example_descr, "Register the image series given by images 'inputXXXX.png' to reference image 30. " 
	 "Skip two images at the beginning and using mutual information as cost function, "
	 "and penalize the transformation by divcurl with weight 5. "
	 "Store the result in 'registeredXXXX.png'."}, 
	{pdi_example_code, "  -i input0000.png -o registered%04d.png -k 2 -r 30 image:cost=mi divcurl:weight=5"}
}; 

C2DFullCostList create_costs(const std::vector<string>& costs, int idx)
{
	stringstream cost_descr; 
	cost_descr << ",src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	C2DFullCostList result; 

	for (auto c = costs.begin(); c != costs.end(); ++c) {
		string cc(*c); 

		if (cc.find("image") == 0) {
			if (cc.find("src") != string::npos  || cc.find("ref") != string::npos) {
				throw create_exception<invalid_argument>( "image cost functions '", cc, 
								"' must not set the 'src' or 'ref' parameter explicitly");
			}
			cc.append(cost_descr.str()); 
		}
		cvdebug() << "create cost:"  << *c << " as " << cc << "\n"; 
		auto imagecost = C2DFullCostPluginHandler::instance().produce(cc);
		result.push(imagecost); 
	}

	return result; 
}

struct SeriesRegistration {
	C2DImageSeries&  input_images; 
	string minimizer; 
	const std::vector<string>& costs; 
	size_t mg_levels; 
	P2DTransformationFactory transform_creator; 
	int reference; 
	
	SeriesRegistration(C2DImageSeries&  _input_images, 
			   const string& _minimizer, 
			   const std::vector<string>& _costs, 
			   size_t _mg_levels, 
			   P2DTransformationFactory _transform_creator, 
			   int _reference):
		input_images(_input_images), 
		minimizer(_minimizer), 
		costs(_costs),
		mg_levels(_mg_levels), 
		transform_creator(_transform_creator), 
		reference(_reference)
		{
		}
	void operator()( const blocked_range<int>& range ) const {
		CThreadMsgStream thread_stream;
		TRACE_FUNCTION; 
		auto m =  CMinimizerPluginHandler::instance().produce(minimizer);

		
		for( int i=range.begin(); i!=range.end(); ++i ) {
			if (i == reference)
				continue; 
			cvmsg() << "Register " << i << " to " << reference << "\n"; 
			auto cost  = create_costs(costs, i); 
			C2DNonrigidRegister nrr(cost, m,  transform_creator,  mg_levels, i);
			P2DTransformation transform = nrr.run(input_images[i], input_images[reference]);
			input_images[i] = (*transform)(*input_images[i]);
		}
	}
};  

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg%04d.v");
	                        
	P2DTransformationFactory transform_creator; 

	// registration parameters
	string minimizer("gsl:opt=gd,step=0.1");
	size_t mg_levels = 3; 
	int reference_param = -1; 
	int skip = 0; 

	int max_threads = task_scheduler_init::automatic;
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', 
				    "file name for registered images, numbering and pattern are deducted from the input data")); 
	
	
	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "spline", "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 
	options.add(make_opt( skip, "skip", 'k', "skip registration of these images at the beginning of the series")); 

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 'T', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  


	if (options.parse(argc, argv, "cost", &C2DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	task_scheduler_init init(max_threads);
	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	P2DImageSeries input_images(new C2DImageSeries); 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P2DImage image = load_image<P2DImage>(src_name);
		if (!image)
			throw create_exception<runtime_error>( "image '", src_name, "' not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images->push_back(image);
	}

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images->size() / 2 : reference_param; 

	if ( input_images->empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images->size() - 1) {
		reference = input_images->size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	SeriesRegistration sreg(*input_images, minimizer, cost_functions, 
				mg_levels, transform_creator, reference); 

	parallel_for(blocked_range<int>( 0, input_images->size()), sreg);

	bool success = true; 
	auto ii = input_images->begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
		cvmsg() << "Save image " << i << " to " << out_name << "\n"; 
		success &= save_image(out_name, *ii); 
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
