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

#define VSTREAM_DOMAIN "2dall2one"

#include <fstream>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>

#include <mia/core.hh>
#include <mia/core/threadedmsg.hh>

#include <mia/internal/main.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/SegSetWithImages.hh>


#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

using namespace tbb;
using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescription g_description = {
        {pdi_group, "Registration of series of 2D images"}, 
	{pdi_short, "Non-linear registration of a series of 2D images."}, 
	{pdi_description, "This program runs non-rigid registration of a series of "
	 "images given in an image set. All images are registered to one user defined reference image."}, 
	{pdi_example_descr, "Register the perfusion series given in segment.set by optimizing a "
	 "spline based transformation with a coefficient rate of 16 pixel using Mutual Information "
	 "and penalize the transformation by using divcurl with aweight of 2.0."}, 
	{pdi_example_code, "-i segment.set -o registered.set -F spline:rate=16 "
	 "image:cost=mi,weight=2.0 divcurl:weight=2.0"}
}; 

C2DFullCostList create_costs(const vector<string>& costs, int idx)
{
	stringstream cost_descr; 
	cost_descr << ",src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	C2DFullCostList result; 

	for (auto c = costs.begin(); c != costs.end(); ++c) {
		string cc(*c); 

		if (cc.find("image") == 0) 
			cc.append(cost_descr.str()); 
		cvdebug() << "create cost:"  << *c << " as " << cc << "\n"; 
		auto imagecost = C2DFullCostPluginHandler::instance().produce(cc);
		result.push(imagecost); 
	}

	return result; 
}

struct SeriesRegistration {
	CSegSetWithImages& input_set; 
	C2DImageSeries& input_images; 
	string minimizer; 
	const vector<string>& costs; 
	size_t mg_levels; 
	P2DTransformationFactory transform_creator; 
	int reference; 

	SeriesRegistration(CSegSetWithImages& _input_set, 
			   C2DImageSeries& _input_images, 
			   const string& _minimizer, 
			   const vector<string>& _costs, 
			   size_t _mg_levels, 
			   P2DTransformationFactory _transform_creator, 
			   int _reference
		):
		input_set(_input_set),
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
		CSegSetWithImages::Frames& frames = input_set.get_frames(); 
		
		
		for( int i=range.begin(); i!=range.end(); ++i ) {
			if (i == reference)
				continue; 
			cvmsg() << "Register " << i << " to " << reference << "\n"; 
			auto cost  = create_costs(costs, i); 
			C2DNonrigidRegister nrr(cost, m,  transform_creator,  mg_levels, i);
			P2DTransformation transform = nrr.run(input_images[i], input_images[reference]);
			input_images[i] = (*transform)(*input_images[i]);
			frames[i].inv_transform(*transform);
		}
	}
};  

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg");
	string out_filename;
	
	P2DTransformationFactory transform_creator; 
	
	// registration parameters
	string minimizer("gsl:opt=gd,step=0.1");
	size_t mg_levels = 3; 
	int reference_param = -1; 
	int skip = 0; 
	
	int max_threads = task_scheduler_init::automatic;
	
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-filebase", 0, "file name basae for registered files, file "
			      "Wtype is deducted from the image file type in the input data set.")); 

	options.set_group("\nRegistration"); 
	options.add(make_opt( skip, "skip", 'k', "Skip images at the beginning of the series"));
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "spline", "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 'T', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  
	

	if (options.parse(argc, argv, "cost", &C2DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	task_scheduler_init init(max_threads);
	
	CSegSetWithImages  input_set(in_filename, true);
	C2DImageSeries input_images = input_set.get_images(); 
	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 
	
	
	// if reference is not given, use half range 
	int reference = reference_param < 0  ? input_images.size() / 2 : reference_param; 
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 
	
	if (reference > static_cast<long>(input_images.size()) - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	if (skip > reference - 1) {
		throw invalid_argument("Skipping past reference\n");  
	}
	
	SeriesRegistration sreg(input_set, input_images, minimizer, cost_functions, 
				mg_levels, transform_creator, reference); 
	
	parallel_for(blocked_range<int>( skip, input_images.size()), sreg);
	
	
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

MIA_MAIN(do_main); 
