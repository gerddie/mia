/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <memory>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/internal/main.hh>


#include "vfluid.hh"
#include "elast.hh"

NS_MIA_USE;
using namespace std; 


const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 
	{pdi_short, "Fluid dynamic 2D registration"}, 
	{pdi_description, "This program is used for non-rigid registration based on fluid dynamics. "
	"It uses SSD as the sole registration criterion."}, 
	{pdi_example_descr, "Register image test.v to image ref.v and write the deformation "
	 "vector field regfield.v. Start registration at the smallest size above 16 pixel."}, 
	{pdi_example_code, "-i test.v -r ref.v -o regfield.v -s 16"}
}; 

inline void register_level(const C2DImage& ModelScale, const C2DImage& RefScale, C2DFVectorfield& Shift,
			   float regrid_thresh,float epsilon, int level, bool elast, float mu, float lambda, 
			   C2DInterpolatorFactory& ipfactory)
{
	FCopy2DImageToFloatRepn converter;
	C2DFImage Model = converter(ModelScale);
	C2DFImage  Ref   = converter(RefScale);

	if (elast) {
		cerr << "elastic registration" << endl;
		TElastReg(Ref,Model,regrid_thresh,level,epsilon,lambda, mu).work(&Shift, ipfactory);
	}else {
		TFluidReg(Ref,Model,regrid_thresh,level,epsilon,lambda, mu).work(&Shift, ipfactory);
	}
}

C2DFVectorfield upscale( const C2DFVectorfield& vf, C2DBounds size)
{
	C2DFVectorfield Result(size);
	float x_mult = float(size.x) / (float)vf.get_size().x;
	float y_mult = float(size.y) / (float)vf.get_size().y;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;

	C2DFVectorfield::iterator i = Result.begin();

	for (unsigned int y = 0; y < size.y; y++){
		for (unsigned int x = 0; x < size.x; x++,++i){
			C2DFVector help(ix_mult * x, iy_mult * y);
			C2DFVector val = vf.get_interpol_val_at(help);
			*i = C2DFVector(val.x * x_mult,val.y * y_mult);
		}
	}
	return Result;
}


int do_main(int argc, char *argv[])
{
	float regrid_thresh = 0.5;

	string ref_filename;
	string src_filename;
	string out_filename;
	string def_filename;

	unsigned int grid_start = 16;
	float epsilon = 0.001;

	bool elastic;
	float mu = 1.0;
	float lambda = 1.0;
	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	
	options.set_group("File-IO"); 
	options.add(make_opt( src_filename, "in-image", 'i', "input (test) image to be registered", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-transformation", 'o', "output transformation comprising the registration", 
			      CCmdOptionFlags::output, &C2DVFIOPluginHandler::instance()));
	options.add(make_opt( def_filename, "out-image", 'd', "output image deformed according to the transformation", 
			      CCmdOptionFlags::output, &imageio));

	options.set_group("Registration parameters"); 
	options.add(make_opt( grid_start, "mgstart", 'm', "multigrid start size"));
	options.add(make_opt( epsilon, "epsilon", 'e', "optimization breaking condition"));
	options.add(make_opt( mu, "mu", 0, "elasticity parameter"));
	options.add(make_opt( lambda, "lambda", 0, "elasticity parameter"));
	options.add(make_opt( elastic, "elastic", 0, "use elastic registration"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	if (out_filename.empty() && def_filename.empty()) {
		throw invalid_argument("At least one output file must be given"); 
	}


	P2DImage Model = load_image<P2DImage>(src_filename);
	P2DImage Reference = load_image<P2DImage>(ref_filename);

	C2DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size()){
		throw std::invalid_argument("Images have different size");
	}

	time_t start_time = time(NULL);

	unsigned int x_shift = 0;
	unsigned int y_shift = 0;


	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3, bc_mirror_on_bounds));


	while (GlobalSize.x >> x_shift > grid_start){
		x_shift++;
	}
	while (GlobalSize.y >> y_shift > grid_start){
		y_shift++;
	}

	C2DFVectorfield result(C2DBounds(GlobalSize.x >> x_shift,GlobalSize.y >> y_shift));

	bool alter = true;
	while (x_shift && y_shift) {
		C2DBounds BlockSize(1 << x_shift, 1 << y_shift);
		cvmsg() << "Blocksize = " << BlockSize.x << "x"<< BlockSize.y << "\n";

		stringstream downscale_descr;
		downscale_descr << "downscale:bx=" << BlockSize.x << ",by=" << BlockSize.y;
		auto downscaler =
			C2DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());

		P2DImage ModelScale = downscaler->filter(*Model);
		P2DImage RefScale   = downscaler->filter(*Reference);

		if (result.get_size() != ModelScale->get_size())
			result = upscale(result, ModelScale->get_size());

		register_level(*ModelScale,*RefScale,result,regrid_thresh,epsilon,
			       (x_shift >y_shift ? x_shift : y_shift)+1,elastic,mu, lambda, *ipfactory);

		if (alter)
			alter = !alter;
		else {
			if (x_shift)
				x_shift--;
			if (y_shift)
				y_shift--;
		}


	}


	//final Registration at pixel-level
	cvmsg() << "Finales Level" << result.get_size() << "\n";

	result = upscale(result, Model->get_size());
	register_level(*Model,*Reference,result,regrid_thresh,epsilon,1,elastic,mu, lambda, *ipfactory);

	cvmsg() << "Gesamtzeit: " << time(NULL)-start_time << "\n";

	auto vftranscreator  = produce_2dtransform_factory("vf:imgkernel=[bspline:d=1],imgboundary=zero");
	auto trans = vftranscreator->create(result.get_size()); 
	CDoubleVector buffer(trans->degrees_of_freedom(), false);
	
	auto ib = buffer.begin(); 
	for (auto ivf = result.begin(); ivf != result.end(); ++ivf) {
		cvdebug() << *ivf << "\n"; 
		*ib++ = ivf->x; 
		*ib++ = ivf->y; 
	}
	
	trans->set_parameters(buffer); 
	
	// write result deformation field
	if (!out_filename.empty()) {
		if (!C2DTransformationIOPluginHandler::instance().save(out_filename, *trans)) 
			throw create_exception<runtime_error>( "Unable to save transformation to '", out_filename, "'"); 
	}

	if (!def_filename.empty()) {
		P2DImage deformed_image = (*trans)(*Model);
		if (!save_image(def_filename, deformed_image)) 
			throw create_exception<runtime_error>("Unable to save deformed image to '", def_filename, "'");
	}

	return EXIT_SUCCESS;

}


MIA_MAIN(do_main); 
