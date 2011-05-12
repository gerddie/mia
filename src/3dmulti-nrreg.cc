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
  // currently not compiled 
  Latex BeginProgramDescription{3D image registration}

  \subsection{mia-3dmulti-nrreg}
  \label{mia-3dmulti-nrreg}
  
  \begin{description}
  \item [Description:]   This program implements 3D gray scale image registration 
  by optimizing a dense vector field that defines a transformation for each pixel. 
  The input images must be of the same size and dimensions. 
  The registration can be achieved by optimizing a combined cost function. 

  \end{description}

  LatexEnd
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

#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/minmax_element.hpp>
#include <mia/3d/multireg.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const char *g_description = 
	"This program implements non-rigid registration of 3D image\n"
	" using multiple cost functions\n"
	"Basic usage:\n"
	"  mia-3dmulti-nrreg [options] <cost functions>\n";

// set op the command line parameters and run the registration
int do_main(int argc, const char **argv)
{

	CCmdOptionList options(g_description);
	string out_filename;
	string regmodel("navier");
	string timestep("fluid");
	int start_size = 16;
	EInterpolation interpolator = ip_bspline3;
	int max_iter = 200;
	float epsilon = 0.01;

	options.push_back(make_opt( out_filename, "out-file", 'o', "output vector field", CCmdOption::required));
	options.push_back(make_opt( regmodel, "regmodel", 'm', "registration model"));
	options.push_back(make_opt( timestep, "timestep", 't', "time setp"));
	options.push_back(make_opt( start_size, "mgsize", 's', "multigrid start size"));
	options.push_back(make_opt( max_iter, "max-iter", 'n', ",maximum number of iterations"));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator"));
	options.push_back(make_opt( epsilon, "epsilon", 'e', "relative accuracy to stop registration "
				    "at a multi-grid level"));

	options.parse(argc, argv, true);

	if (out_filename.empty()) {
		cvfatal() << "No output filename given\n";
		return EXIT_FAILURE;
	}

	vector<const char *> cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cvfatal() << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}

	P3DRegModel model = C3DRegModelPluginHandler::instance().produce(regmodel.c_str());
	P3DRegTimeStep time_step = C3DRegTimeStepPluginHandler::instance().produce(timestep.c_str());

	if (!model || !time_step)
		return EXIT_FAILURE;


	std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(interpolator));
	if (!ipf)
		throw invalid_argument("unknown interpolator requested");

	C3DImageFatCostList cost_list;
	for(vector<const char *>::const_iterator i = cost_chain.begin(); i != cost_chain.end(); ++i) {
		P3DImageFatCost c = C3DFatImageCostPluginHandler::instance().produce(*i);
		if (c)
			cost_list.push_back(c);
	}
	if (cost_list.empty()) {
		cerr << "Could not create a single cost function\n";
		return EXIT_FAILURE;
	}

	C3DMultiImageRegister reg(start_size, max_iter, *model,  *time_step, epsilon);

	if (!reg(cost_list, ipf)->save(out_filename, "")) {
		cerr << "Unable to save result vector field to " << out_filename << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// for readablility the real main function encapsulates the do_main in a try-catch block
int main(int argc, const char **argv)
{
	try {
		return do_main(argc, argv);
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
