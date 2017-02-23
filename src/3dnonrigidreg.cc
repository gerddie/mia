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

#include <sstream>
#include <mia/core.hh>
#include <mia/internal/main.hh>
#include <mia/3d.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/core/minimizer.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/transformio.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Non-linear registration of 3D images"}, 
	{pdi_description, "This program implements the registration of two gray scale 3D images."}, 
	{pdi_example_descr, "Register image test.v to image ref.v by using a spline transformation with a "
	 "coefficient rate of 5  and write the registered image to reg.v. "
	 "Use two multiresolution levels, ssd as image cost function and divcurl weighted by 10.0 "
	 "as transformation smoothness penalty. "}, 
	{pdi_example_code, "-i test.v -r ref.v -o reg.v -l 2 -f spline:rate=3  image:cost=ssd divcurl:weight=10"}
};

int do_main( int argc, char *argv[] )
{
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	PMinimizer minimizer; 

	cvdebug() << "auto transform_creator\n"; 
	P3DTransformationFactory transform_creator; 


	const auto& image3dio =  C3DImageIOPluginHandler::instance(); 
	const auto& transform3dio =  C3DTransformationIOPluginHandler::instance(); 
	size_t mg_levels = 3;

	CCmdOptionList options(g_description);
	options.set_group("IO"); 
	options.add(make_opt( src_filename, "in-image", 'i', "test image", 
			      CCmdOptionFlags::required_input, &image3dio));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image", 
			      CCmdOptionFlags::required_input, &image3dio));
	options.add(make_opt( out_filename, "out-image", 'o', "registered output image", 
			      CCmdOptionFlags::required_output, &image3dio));
	options.add(make_opt( trans_filename, "transformation", 't', "output transformation", 
			      CCmdOptionFlags::output, &transform3dio));
	
	options.set_group("Registration"); 
	options.add(make_opt( mg_levels, "levels", 'l', "multi-resolution levels"));
	options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "spline:rate=10", "transForm", 'f', "transformation type"));

	if (options.parse(argc, argv, "cost", &C3DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
	auto cost_descrs = options.get_remaining(); 

	C3DFullCostList costs; 
	for (auto i = cost_descrs.begin(); i != cost_descrs.end(); ++i)
		costs.push(C3DFullCostPluginHandler::instance().produce(*i)); 

	P3DImage Model = load_image<P3DImage>(src_filename);
	P3DImage Reference = load_image<P3DImage>(ref_filename);
	C3DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size())
		throw std::invalid_argument("Images have different size");

	C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	P3DTransformation transform = nrr.run(Model, Reference);
	P3DImage result = (*transform)(*Model);

	if (!trans_filename.empty()) {
		if (!C3DTransformationIOPluginHandler::instance().save(trans_filename, *transform)) 
			cverr() << "Saving the transformation to '" << trans_filename << "' failed."; 
	}

	return save_image(out_filename, result);
}

MIA_MAIN(do_main); 
