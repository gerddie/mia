/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

/*
  This program implements 2D gray scale image registration. 
  Depending on the available plug-ins it can accomodate various models and cost-functions. 
  So far input images can be given as PNG, TIF, BMP and OpenEXR (depending on the installed plug-ins)
  The input images must be of the same dimensions and gray scale (whatever bit-depth). 
  The vector field will be saved as a EXR image with two frames of float values, one for the X 
  and one for the Y values of the vectors. 
  Other vector field output plug-ins might be supported depending on the installed plug-ins. 
*/

#include <mia/core.hh>
#include <mia/2d.hh>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/minmax_element.hpp>

NS_MIA_USE
using namespace boost; 
using namespace std; 

// set op the command line parameters and run the registration 
int do_main(int argc, const char **args)
{

	CCmdOptionList options;
	string src_filename; 
	string out_filename; 
	string def_filename; 
	string ref_filename; 
	string mov_filename; 
	string cost_function("ssd"); 
	string regmodel("navier"); 
	string timestep("fluid"); 
	int start_size = 16; 
	EInterpolation interpolator = ip_bspline3; 
	int max_iter = 200; 
	float epsilon = 0.01; 
	bool save_steps = false; 

	const C2DImageCostPluginHandler::Instance& icph = C2DImageCostPluginHandler::instance(); 

	options.push_back(make_opt( src_filename, "in-file", 'i', "input image (floating image)", "template", true));
	options.push_back(make_opt( ref_filename, "ref-file", 'r', "reference image", "reference", true));
	options.push_back(make_opt( def_filename, "def-file", 'd', "deformed inpout image", NULL, false));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output vector field", "output", true));
	options.push_back(make_opt( mov_filename, "mov-file", 0, "movie generator image", NULL, false));
	options.push_back(make_opt( regmodel, "regmodel", 'm', "registration model", "model", false));
	options.push_back(make_opt( timestep, "timestep", 't', "time setp", "timestep", false));
	options.push_back(make_opt( start_size, "mgsize", 's', "multigrid start size", "start", false));
	options.push_back(make_opt( max_iter, "max-iter", 'n', ",maximum number of iterations", "iter", false));
	options.push_back(make_opt( cost_function, "cost", 'c', "cost function", "cost", false)); 
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p', 
					"image interpolator", "interpolator", false)); 
	options.push_back(make_opt( epsilon, "epsilon", 'e', "relative accuracy to stop registration at a multi-grid level", NULL, false)); 
	options.push_back(make_opt( save_steps, "save-steps", 0, "save the steps of the registration in images", NULL, false));

	options.parse(argc, args);

	if (!options.get_remaining().empty()) {
		cerr << "Unknown options found\n"; 
		return EXIT_FAILURE;
	}

	P2DImageCost cost = icph.produce(cost_function.c_str());
	P2DRegModel model = C2DRegModelPluginHandler::instance().produce(regmodel.c_str()); 
	P2DRegTimeStep time_step = C2DRegTimeStepPluginHandler::instance().produce(timestep.c_str()); 
	
	if (!cost || !model || !time_step)
		return EXIT_FAILURE;

	
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance(); 
	typedef C2DImageIOPluginHandler::Instance::PData PImageVector; 


	PImageVector reference = imageio.load(ref_filename); 
	PImageVector source    = imageio.load(src_filename); 
	PImageVector movie; 

	if (!mov_filename.empty() ) {
		movie = imageio.load(mov_filename); 
	}

	if (!movie && save_steps)
		movie = source; 
	
	if (!reference || reference->size() < 1) {
		cerr << "no image found in " << ref_filename << "\n"; 
		return EXIT_FAILURE;
	}

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n"; 
		return EXIT_FAILURE;
	}
	
	SHARED_PTR(C2DInterpolatorFactory) ipf(create_2dinterpolation_factory(interpolator)); 
	if (!ipf)
		throw invalid_argument("unknown interpolator requested"); 

	C2DImageRegister reg(start_size, *cost, max_iter, *model, *time_step, *ipf, epsilon, save_steps, 
			     (!movie || movie->empty()) ?  P2DImage() : *movie->begin());

	P2DFVectorfield regfield = reg(**source->begin(), **reference->begin()); 
	C2DIOVectorfield outfield(*regfield);
	
	 if (!C2DVFIOPluginHandler::instance().save("", out_filename, outfield)){
		cerr << "Unable to save result vector field to " << out_filename << "\n"; 
		return EXIT_FAILURE; 
	}
	
	if (!def_filename.empty()) {
		C2DImageVector vimg;
		vimg.push_back(filter(FDeformer2D(*regfield, *ipf), **source->begin())); 
		if (!imageio.save("", def_filename, vimg)) {
			cerr << "Unable to save result image to " << def_filename << "\n"; 
			return EXIT_FAILURE; 
		}
			
	}
	return EXIT_SUCCESS; 
}

// for readablility the real main function encapsulates the do_main in a try-catch block
int main(int argc, const char **args)
{
	try {
		return do_main(argc, args); 
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n"; 
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n"; 
	}
	catch (std::exception& err) {
		cerr << "exception: " << err.what() << "\n"; 
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE; 
}
