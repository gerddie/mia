/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <boost/lambda/lambda.hpp>
#include <mia/core.hh>

#include <libxml++/libxml++.h>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ground_truth_evaluator.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/similarity_profile.hh>


using namespace std;
using namespace mia;

const char *g_description = 
	"This program is used to evaluate an estimate of Ground Thruth for motion compensation "
	"of series of myocardial perfusion images as decribed in Chao Li and Ying Sun, "
	"'Nonrigid Registration of Myocardial Perfusion MRI Using Pseudo Ground Truth' , In Proc. "
	" Medical Image Computing and Computer-Assisted Intervention – MICCAI 2009, 165-172, 2009 "; 

struct SPGTParams {
	double alpha; 
	double beta; 
	double rho_thresh;

	SPGTParams(); 
}; 

const TDictMap<EMinimizers>::Table g_minimizer_table[] = {
	{"cg-fr", min_cg_fr},
	{"cg-pr", min_cg_pr},
	{"bfgs", min_bfgs},
	{"bfgs2", min_bfgs2},
	{"gd", min_gd},
	{NULL, min_undefined}
};


struct SRegistrationParams { 
	EMinimizers minimizer;
	size_t c_rate; 
	double divcurlweight; 
	double imageweight; 
	shared_ptr<C2DInterpolatorFactory> ipfactory; 
	size_t mg_levels; 
	SRegistrationParams(); 
}; 

SPGTParams::SPGTParams():
	alpha(1.0), 
	beta(1.0), 
	rho_thresh(0.9)
{
}
	
SRegistrationParams::SRegistrationParams():
	minimizer(min_gd), 
	c_rate(32), 
	divcurlweight(20.0), 
	imageweight(1.0), 
	mg_levels(3)
{
}

C2DFullCostList create_costs(double divcurlweight, double imageweight)
{
	C2DFullCostList result; 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	stringstream image_descr; 
	image_descr << "image:cost=ssd,weight=" << imageweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(image_descr.str())); 
	return result; 
}

P2DTransformationFactory create_transform_creator(size_t c_rate)
{
	stringstream transf; 
	transf << "spline:rate=" << c_rate; 
	return C2DTransformCreatorHandler::instance().produce(transf.str()); 
}


void run_registration_pass(CSegSetWithImages&  input_set, const C2DImageSeries& references, 
			   int skip_images, const SRegistrationParams& params) 
{
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	C2DImageSeries input_images = input_set.get_images(); 
	auto costs  = create_costs(params.divcurlweight, params.imageweight); 
	auto transform_creator = create_transform_creator(params.c_rate); 
	C2DNonrigidRegister nrr(costs, params.minimizer,  transform_creator, 
				*params.ipfactory, params.mg_levels);

	// this loop could be parallized 
	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register frame " << i << "\n"; 
		P2DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
		input_images[i + skip_images] = (*transform)(*input_images[i + skip_images], *params.ipfactory);
		frames[i + skip_images].inv_transform(*transform);
	}
	input_set.set_images(input_images); 
}


int do_main( int argc, const char *argv[] )
{
	SPGTParams pgt_params; 
	
	SRegistrationParams reg_params; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	
	string in_filename;
	string out_filename;
	string registered_filebase("reg");

	size_t skip = 2; 
	size_t passes = 4; 

	double c_rate_divider = 4; 
	double divcurlweight_divider = 4.0; 

	EInterpolation interpolator = ip_bspline3;

	CCmdOptionList options(g_description);
	options.set_group("\nFile-IO"); 
	options.push_back(make_opt( in_filename, "in-file", 'i', "input perfusion data set", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output perfusion data set", "output", true));
	options.push_back(make_opt( registered_filebase, "registered", 'r', "file name base for registered fiels", 
				    "registered", false)); 

	options.set_group("\nPreconditions"); 
	options.push_back(make_opt( skip, "skip", 's', "skip images at beginning of series", "skip", false));
	options.push_back(make_opt( passes, "passes", 'P', "number of registration passes", "passes", false));

	options.set_group("\nPseudo-Ground-Thruth"); 
	options.push_back(make_opt( pgt_params.alpha, "alpha", 'a', "spacial neighborhood penalty weight", 
				    "alpha", false));
	options.push_back(make_opt( pgt_params.beta, "beta", 'b', "temporal second derivative penalty weight", 
				    "beta", false));
	options.push_back(make_opt( pgt_params.rho_thresh, "rho_thresh", 'r', 
				    "crorrelation threshhold for neighborhood analysis", "rho", false));


	
	options.set_group("\nRegistration"); 
	
	options.push_back(make_opt( reg_params.minimizer, TDictMap<EMinimizers>(g_minimizer_table),
				    "optimizer", 'O', "Optimizer used for minimization", 
				    "optimizer", false));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.push_back(make_opt( reg_params.mg_levels, "mr-levels", 'l', "multi-resolution levels", 
				    "mg-levels", false));
	
	options.push_back(make_opt( reg_params.divcurlweight, "divcurl", 'd', 
				    "divcurl regularization weight", "divcurl", false));
	options.push_back(make_opt( divcurlweight_divider, "divcurl-divider", 0,
				    "divcurl weight scaling with each new pass", 
				    "divcurl", false)); 
	options.push_back(make_opt( reg_params.c_rate, "start-c-rate", 'a', 
				    "start coefficinet rate in spines, gets divided by --c-rate-divider with every pass", 				    "c-rate", false));
	options.push_back(make_opt( c_rate_divider, "c-rate-divider", 0, 
				    "cofficient rate divider for each pass", 
				    "c-rate", false));
	options.push_back(make_opt( reg_params.imageweight, "imageweight", 'w', 
				    "image cost weight", "imageweight", false)); 

	options.parse(argc, argv, false);

	
	reg_params.ipfactory.reset(create_2dinterpolation_factory(interpolator));
		
	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	
	if (skip >= input_set.get_frames().size()) {
		THROW(invalid_argument, "Try to skip " << skip 
		      << " images, but input set has only " << input_set.get_frames().size() << " images.");  
	}
	
	// create ground thruth evaluator 
	C2DGroundTruthEvaluator gte(pgt_params.alpha, pgt_params.beta, pgt_params.rho_thresh);
	vector<P2DImage> pgt;
	
	// main registration loop 
	size_t pass = 0; 
	while (pass < passes) {
		cvmsg() << "Registration pass " << pass << "\n"; 
		// skip images at start 
		C2DImageSeries images = input_set.get_images(); 
		C2DImageSeries series(images.begin() + skip, images.end()); 

		// create pseudeo ground thruth 
		gte(series, pgt);

		// run registration pass 
		run_registration_pass(input_set, pgt, skip, reg_params); 
		
		// prepare next pass 
		reg_params.c_rate /= c_rate_divider; 
		reg_params.divcurlweight /= divcurlweight_divider; 
		++pass; 
	}

	// copy back registered images
	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

}

int main( int argc, const char *argv[] )
{
	try {
		return do_main(argc, argv);
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}

