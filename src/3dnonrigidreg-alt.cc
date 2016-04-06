/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <sstream>
#include <mia/core.hh>
#include <mia/3d.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/core/minimizer.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/transformio.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	{pdi_group,  "Registration, Comparison, and Transformation of 3D images"}, 

	{pdi_short,  "Non-linear registration of 3D images."},
	
	{pdi_description, "This program runs a non-rigid registration based on the given cost criteria "
	 "and a given transformation model. Other than mia-3dnonrigidreg it doesn't support "
	 "specific command line parameters to provide the images. Instead the images are specified "
	 "dirctly when defining the cost function. Hence, image registrations can be executed that "
	 "optimize the aligmnet of  more than one image pair at the same time. Note, however, that "
	 "all input images must be of the same dimension (in pixels)"}, 
	
	{pdi_example_descr,
	 "Register image test.v to image ref.v by using a spline transformation with a "
	 "coefficient rate of 5  and write the registered image to reg.v. "
	 "Use two multiresolution levels, ssd as image cost function and divcurl weighted by 10.0 "
	 "as transformation smoothness penalty. The resulting transformation is saved in reg.vf."},

	{pdi_example_code, "-o reg.vf -l 2 -f spline:rate=3 image:cost=ssd,src=test.v,ref=ref.v divcurl:weight=10"}
};


int do_main( int argc, char *argv[] )
{
	string trans_filename;
	size_t mg_levels = 3;
	PMinimizer minimizer; 
	P3DTransformationFactory transform_creator; 

	const auto& transform3dio =  C3DTransformationIOPluginHandler::instance(); 

	CCmdOptionList options(g_description);
	options.add(make_opt( trans_filename, "out-transform", 'o', "output transformation", 
			      CCmdOptionFlags::required_output, &transform3dio));
	options.add(make_opt( mg_levels, "levels", 'l', "multi-resolution levels"));
	options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "spline:rate=10", "transForm", 'f', "transformation type"));
	
	if (options.parse(argc, argv, "cost", &C3DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
	auto cost_descrs = options.get_remaining(); 

	C3DFullCostList costs; 
	for (auto i = cost_descrs.begin(); i != cost_descrs.end(); ++i)
		costs.push(C3DFullCostPluginHandler::instance().produce(*i)); 


	C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	P3DTransformation transform = nrr.run();

	if (! transform3dio.save(trans_filename, *transform) )
		throw create_exception<runtime_error>("Unable to save obtained transformation to '", trans_filename, "'");
	return EXIT_SUCCESS; 
}



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
