/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
  This program implements 3D gray scale image registration.
  Depending on the available plug-ins it can accomodate various models and cost-functions.
  So far input images can be given as PNG, TIF, BMP and OpenEXR (depending on the installed plug-ins)
  The input images must be of the same dimensions and gray scale (whatever bit-depth).
  The vector field will be saved as a EXR image with two frames of float values, one for the X
  and one for the Y values of the vectors.
  Other vector field output plug-ins might be supported depending on the installed plug-ins.
*/

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>

#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const char *g_description = 
	"This program implements 3D gray scale image registration but\n"
	"is currently not funcioning\n"
	"Basic usage:\n"
	"  mia-3dnrreg [options] \n";


// set op the command line parameters and run the registration
int do_main(int argc, const char **args)
{
	CCmdOptionList options(g_description);

	string src_filename;
	string out_filename;
	string def_filename;
	string ref_filename;
	string cost_function("ssd");
	string regmodel("navier");
	string timestep("fluid");
	int start_size = 16;
	EInterpolation interpolator = ip_bspline3;
	int max_iter = 200;
	float epsilon = 0.01;
	bool save_steps = false;

	const C3DImageCostPluginHandler::Instance&  icph = C3DImageCostPluginHandler::instance();

	options.push_back(make_opt( src_filename, "in-file", 'i', "input image (floating image)", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref-file", 'r', "reference image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output vector field", CCmdOption::required));
	options.push_back(make_opt( def_filename, "def-file", 'd', "deformed inpout image"));
	options.push_back(make_opt( regmodel, "regmodel", 'm', "registration model", CCmdOption::required));
	options.push_back(make_opt( timestep, "timestep", 't', "time setp", CCmdOption::required));
	options.push_back(make_opt( start_size, "mgsize", 's', "multigrid start size"));
	options.push_back(make_opt( max_iter, "max-iter", 'n', ",maximum number of iterations"));
	options.push_back(make_opt( cost_function, "cost", 'c', "cost function", CCmdOption::required));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
					"image interpolator", CCmdOption::required));
	options.push_back(make_opt( epsilon, "epsilon", 'e', "relative accuracy to stop registration"
				    " at a multi-grid level", CCmdOption::required));
	options.push_back(make_opt( save_steps, "save-steps", 0, "save the steps of the registration in images"));

	options.parse(argc, args);

	if (! options.get_remaining().empty()) {
		cerr << "Unknown options found\n";
		return EXIT_FAILURE;
	}

	P3DImageCost cost = icph.produce(cost_function.c_str());
	P3DRegModel model = C3DRegModelPluginHandler::instance().produce(regmodel.c_str());
	P3DRegTimeStep time_step = C3DRegTimeStepPluginHandler::instance().produce(timestep.c_str());


	if (src_filename.empty() || ref_filename.empty() || out_filename.empty()) {
		cerr << "input, reference and output file names are required\n";
		return EXIT_FAILURE;
	}

	if (!cost || !model || !time_step)
		return EXIT_FAILURE;


	const C3DImageIOPluginHandler::Instance&  imageio = C3DImageIOPluginHandler::instance();
	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;


	PImageVector reference = imageio.load(ref_filename);
	PImageVector source    = imageio.load(src_filename);

	if (!reference || reference->size() < 1) {
		cerr << "no image found in " << ref_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(interpolator));
	if (!ipf)
		throw invalid_argument("unknown interpolator requested");

	C3DImageRegister reg(start_size, *cost, max_iter, *model, *time_step, *ipf, epsilon, save_steps);

	P3DFVectorfield regfield = reg(**source->begin(), **reference->begin());
	C3DIOVectorfield outfield(regfield->get_size());
	copy(regfield->begin(), regfield->end(), outfield.begin());

	if (!C3DVFIOPluginHandler::instance().save("", out_filename, outfield)){
		cerr << "Unable to save result vector field to " << out_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!def_filename.empty()) {
		C3DImageVector vimg;
		vimg.push_back(filter(FDeformer3D(*regfield, *ipf), **source->begin()));
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
