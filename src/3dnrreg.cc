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

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>

#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
	{pdi_group, "Work in progress"},
	
	{pdi_short, "Non-linear registration of 3D images."}, 

	{pdi_description, "This program implements 3D gray scale image registration but "
	 "is currently not funcioning"}
}; 


// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
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
	PSplineKernel interpolator_kernel;
	int max_iter = 200;
	float epsilon = 0.01;
	bool save_steps = false;

	const C3DImageCostPluginHandler::Instance&  icph = C3DImageCostPluginHandler::instance();

	options.add(make_opt( src_filename, "in-file", 'i', "input image (floating image)", CCmdOptionFlags::required_input));
	options.add(make_opt( ref_filename, "ref-file", 'r', "reference image", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', "output vector field", CCmdOptionFlags::required_output));
	options.add(make_opt( def_filename, "def-file", 'd', "deformed output image", CCmdOptionFlags::output));
	options.add(make_opt( regmodel, "regmodel", 'm', "registration model"));
	options.add(make_opt( timestep, "timestep", 't', "time setp"));
	options.add(make_opt( start_size, "mgsize", 's', "multigrid start size"));
	options.add(make_opt( max_iter, "max-iter", 'n', ",maximum number of iterations"));
	options.add(make_opt( cost_function, "cost", 'c', "cost function"));
	options.add(make_opt( interpolator_kernel, "bspline:d=3", "interpolator", 'p', "image interpolator kernel"));
	options.add(make_opt( epsilon, "epsilon", 'e', "relative accuracy to stop registration"
				    " at a multi-grid level"));
	options.add(make_opt( save_steps, "save-steps", 0, "save the steps of the registration in images"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

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

	C3DInterpolatorFactory ipfactory(interpolator_kernel, "mirror");

	C3DImageRegister reg(start_size, *cost, max_iter, *model, *time_step, ipfactory, epsilon, save_steps);

	P3DFVectorfield regfield = reg(**source->begin(), **reference->begin());
	C3DIOVectorfield outfield(regfield->get_size());
	copy(regfield->begin(), regfield->end(), outfield.begin());

	if (!C3DVFIOPluginHandler::instance().save(out_filename, outfield)){
		cerr << "Unable to save result vector field to " << out_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!def_filename.empty()) {
		C3DImageVector vimg;
		vimg.push_back(filter(FDeformer3D(*regfield, ipfactory), **source->begin()));
		if (!imageio.save(def_filename, vimg)) {
			cerr << "Unable to save result image to " << def_filename << "\n";
			return EXIT_FAILURE;
		}

	}
	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
