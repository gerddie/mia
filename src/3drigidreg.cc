/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <mia/core/minimizer.hh>
#include <mia/3d.hh>
#include <mia/3d/rigidregister.hh>
#include <mia/3d/transformio.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	{pdi_group,
	 "Registration, Comparison, and Transformation of 3D images"}, 
	 
	{pdi_short,
	 "Linear registration of 3D images."}, 

	{pdi_description,	
	 "This program implements the registration of two gray scale 3D images. "
	 "The transformation is not penalized, therefore, one should only use translation, rigid, or affine "
	 "transformations as target and run mia-3dnonrigidreg of nonrigid registration is to be achieved."}, 
	 
	{pdi_example_descr,	
	 "Register image test.v to image ref.v affine and write the registered image to reg.v. "
	 "Use two multiresolution levels and ssd as cost function."},
	
	{pdi_example_code,
	 "-i test.v -r ref.v -o reg.v -l 2 -f affine -c ssd"}
}; 


int do_main( int argc, char *argv[] )
{
	P3DImageCost cost_function; 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	P3DTransformationFactory transform_creator; 
	PMinimizer minimizer; 

	size_t mg_levels = 3;

	CCmdOptionList options(g_description);
	options.set_group("File I/O"); 
	options.add(make_opt( src_filename, "in-image", 'i', "test image", 
			      CCmdOptionFlags::required_input, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image", 
			      CCmdOptionFlags::required_input, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( out_filename, "out-image", 'o', "registered output image", 
			      CCmdOptionFlags::required_output, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( trans_filename, "transformation", 't', "transformation output file name", 
			      CCmdOptionFlags::output, &C3DTransformationIOPluginHandler::instance() ));

	
	options.add(make_opt( cost_function, "ssd", "cost", 'c', "cost function")); 
	options.add(make_opt( mg_levels, "levels", 'l', "multigrid levels"));
	options.add(make_opt( minimizer, "gsl:opt=simplex,step=1.0", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "rigid",  "transForm", 'f', "transformation type"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// sanity checks: These functions will throw if no plugin is found. 
	if (!trans_filename.empty())
		C3DTransformationIOPluginHandler::instance().preferred_plugin(trans_filename); 
	C3DImageIOPluginHandler::instance().preferred_plugin(out_filename);
	

	P3DImage Model = load_image<P3DImage>(src_filename);
	P3DImage Reference = load_image<P3DImage>(ref_filename);

	C3DRigidRegister rr(cost_function, minimizer,  transform_creator, mg_levels);

	P3DTransformation transform = rr.run(Model, Reference);
	P3DImage result = (*transform)(*Model);

	if (!trans_filename.empty()) {
		cvmsg() << "Save transformation to file '" << trans_filename << "'\n"; 
		if (!C3DTransformationIOPluginHandler::instance().save(trans_filename, *transform)) 
			cverr() << "Saving the transformation to '" << trans_filename << "' failed."; 
	}

	return save_image(out_filename, result);
}



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
