/*
** Copyright (c) Leipzig, Madrid 1999-2010
**                    Gert Wollny <wollny@cns.mpg.de>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include <mia/2d/transformfactory.hh>
#include <mia/2d/deformer.hh>


#include "vfluid.hh"
#include "elast.hh"

NS_MIA_USE;

const char *g_description = "This code implements non-rigid image registration "
	"by implementing a fluid-dynamic transformation model."; 

inline void register_level(const C2DImage& ModelScale, const C2DImage& RefScale, C2DFVectorfield& Shift,
			   float regrid_thresh,float epsilon, int level, bool elast, float lambda, float mu,
			   C2DInterpolatorFactory& ipfactory)
{
	FConvert2DImage2float converter;
	C2DFImage Model = ::mia::filter(converter, ModelScale);
	C2DFImage  Ref   = ::mia::filter(converter, RefScale);

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


int main(int argc, const char *args[])
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

        try {

		CCmdOptionList options(g_description);
		options.push_back(make_opt( src_filename, "in-image", 'i', "input (model) image to be registered", 
					    CCmdOption::required));
		options.push_back(make_opt( ref_filename, "ref-image", 'r', "reference image"));
		options.push_back(make_opt( out_filename, "out", 'o', "output vector field"));
		options.push_back(make_opt( def_filename, "deformed-image", 'd', "deformed registered image"));

		options.push_back(make_opt( grid_start, "mgstart", 'm', "multigrid start size"));
		options.push_back(make_opt( epsilon, "epsilon", 'e', "optimization breaking condition"));
		options.push_back(make_opt( mu, "mu", 0, "elasticity parameter"));
		options.push_back(make_opt( lambda, "lambda", 0, "elasticity parameter"));
		options.push_back(make_opt( elastic, "elastic", 0, "use elastic registration"));

		options.parse(argc, args);


		P2DImage Model = load_image<P2DImage>(src_filename);
		P2DImage Reference = load_image<P2DImage>(ref_filename);

		C2DBounds GlobalSize = Model->get_size();
		if (GlobalSize != Reference->get_size()){
			throw std::invalid_argument("Images have different size");
		}

		time_t start_time = time(NULL);

		unsigned int x_shift = 0;
		unsigned int y_shift = 0;


		auto_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));


		while (GlobalSize.x >> x_shift > grid_start){
			x_shift++;
		}
		while (GlobalSize.y >> y_shift > grid_start){
			y_shift++;
		}

		C2DFVectorfield transform(C2DBounds(GlobalSize.x >> x_shift,GlobalSize.y >> y_shift));

		bool alter = true;
		while (x_shift && y_shift) {
			C2DBounds BlockSize(1 << x_shift, 1 << y_shift);
			cvmsg() << "Blocksize = " << BlockSize.x << "x"<< BlockSize.y << "\n";

			stringstream downscale_descr;
			downscale_descr << "downscale:bx=" << BlockSize.x << ",by=" << BlockSize.y;
			C2DFilterPlugin::ProductPtr downscaler =
				C2DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());

			P2DImage ModelScale = downscaler->filter(*Model);
			P2DImage RefScale   = downscaler->filter(*Reference);

			if (transform.get_size() != ModelScale->get_size())
				transform = upscale(transform, ModelScale->get_size());

			register_level(*ModelScale,*RefScale,transform,regrid_thresh,epsilon,
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
		cvmsg() << "Finales Level" << transform.get_size() << "\n";

		transform = upscale(transform, Model->get_size());
		register_level(*Model,*Reference,transform,regrid_thresh,epsilon,1,elastic,mu, lambda, *ipfactory);

		cvmsg() << "Gesamtzeit: " << time(NULL)-start_time << "\n";

		// write result deformation field
		if (!out_filename.empty()) {
			C2DIOVectorfield outfield(transform);
			if (!C2DVFIOPluginHandler::instance().save("", out_filename, outfield)){
				THROW(runtime_error, "Unable to save result field to '" << out_filename << "'");
			}
		}

		if (!def_filename.empty()) {
			FDeformer2D deformer(transform, *ipfactory);
			P2DImage result = ::mia::filter(deformer, *Model);
			if (!save_image(def_filename, result))
				THROW(runtime_error, "Unable to save result to '" << def_filename << "'");
		}

		return EXIT_SUCCESS;

	}
	catch(std::invalid_argument& x) {
		cverr()<< args[0] << ": invalid argument: "<< x.what() << "\n";
	}
	catch(std::exception& x) {
		cverr()<< args[0] << ": exception " << x.what() << "\n";
	}
	return EXIT_FAILURE;
}

