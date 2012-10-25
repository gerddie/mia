/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>
#include <list>
#include <string>
#include <mia/core.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/internal/main.hh>

#include "vfluid.hh"
#include "sor_solver.hh"


using namespace mia;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Fluid dynamic 3D registration"}, 
	{pdi_description, "This program is used for non-rigid registration based on fluid dynamics as described in: "
	 "Wollny, G. and Kruggel, F., 'Computational cost of non-rigid registration algorithms "
	 "based on fluid dynamics', IEEE Transactions on Medical Imaging, 11(8), pp. 946-952, 2002, "
	 "doi:10.1109/TMI.2002.803113. It uses SSD as the sole registration criterion."}, 
	{pdi_example_descr, "Register image test.v to image ref.v and write the deformation vector "
	 "field regfield.v. Start registration at the smallest size above 16 pixel."}, 
	{pdi_example_code, "-i test.v -r ref.v -o regfield.v -s 16"}
}; 

#define MU	  1	// Lame elasticity constants
#define LAMBDA	  1	//

enum TMethod { meth_sor,
	       meth_cg,
	       meth_filter,
	       meth_sorex,
	       meth_sorap
};


const TDictMap<TMethod>::Table method_dict[] = {
	{"sor", meth_sor, "succesive overrelaxation"}
	,{"sora",meth_sorex, "Gauss Southwell relexation"}
//	,{"sorap",meth_sorap}
	,{NULL, meth_sor, ""}
};

const TDictMap<TMethod> g_method_dict(method_dict);


TMeasureList g_MeasureList;
char StatFileName[1024];

template <class T>

struct delta : public binary_function<T, T, T> {
	T operator()(T x, T y) { return fabs(x-y); }
};


#if 0
void WriteStats(FILE *f,double wholetime,const TFluidRegParams& params,int method,const C3DFVectorfield& Shift)
{


	fprintf(f,"# Size           my     lambda                 ==========\n");
	fprintf(f,"(%3d,%3d,%3d) %8.5f %8.5f \n",
		Shift.get_size().x,
		Shift.get_size().y,
		Shift.get_size().z,
		params.My,
		params.Lambda
		);

	char help[100];
	switch (method) {
	case meth_cg:  fprintf(f,"cg %hd %e\n",params.maxiter,params.factor);break;
	case meth_filter:fprintf(f,"co %d\n",2*params.HalfFilterDim+1);break;
	case meth_sorex:fprintf(f,"ex %f %hd %e\n",
			       params.Overrelaxation,params.maxiter,params.factor);break;
	case meth_sor: fprintf(f,"so %f %hd %e\n",
			       params.Overrelaxation,params.maxiter,params.factor);break;
	default:strcpy(help,"unknown");
	}


	fprintf(f,"#    Size      PDE time  PDE Eval  PDE Time/Eval Regrids niter/Eval timeFromStart\n");

	for (TMeasureList::const_iterator i = g_MeasureList.begin();
	     i != g_MeasureList.end();i++) {
		fprintf(f,"(%3d,%3d,%3d)  %8.5f  %8d  %12.5f %7d %7d %12.5f\n",
			(*i).Size.x,
			(*i).Size.y,
			(*i).Size.z,
			(*i).PDETime,
			(*i).PDEEval,
			(*i).PDETime/(*i).PDEEval,
			(*i).Regrids,(*i).niter/(*i).PDEEval,
			(*i).allovertime
			);
	}



	FDeformer3D deformer(*current_shift, ipf);
	P3DImage ModelDeformed = ::mia::filter(deformer, *ModelScale);


	C3DFImage Help = transform3d<C3DFImage>( params.source->get_size(),
						T3DTriLinInterpolator<C3DFImage>(*params.source),
								       Shift);

	double mismatch = inner_product(Help.begin(), Help.end(),
					params.reference->begin(),
					0.0f , plus<float>(), delta<float>());


	fprintf(f,"%f %f %f\n",wholetime,
		inner_product(params.source->begin(), params.source->end(),
			      params.reference->begin(),
			      0.0f, plus<float>(), delta<float>()),
		mismatch);
}

#endif

int do_main(int argc, char *argv[])
{
	//	FILE *srcf,*reff,*outf;

	TMethod method = meth_sorex;

	TLinEqnSolver *solver=NULL;

    	TFluidRegParams params;
	
	PSplineKernel interpolator_kernel;

	params.InitialStepsize = MAX_STEP;
	params.Lambda = LAMBDA;
	params.My = MU;
	params.HalfFilterDim = 3;
	params.Overrelaxation = 1.0;
	params.useMutual = false;
	params.maxiter = 10;
	params.factor = 0.01;
	params.checkerboard = false;
	params.matter_threshold = 4.0;


	string in_filename;
	string out_filename;
	string ref_filename;
	string statlog_filename;

	bool disable_multigrid = false;
	bool disable_fullres = false;

	CCmdOptionList options(g_description);

	const auto& imageio = C3DImageIOPluginHandler::instance();

	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-image", 'i', "input image", CCmdOption::required, &imageio ));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image ", CCmdOption::required, &imageio ));
	options.add(make_opt( out_filename, "out-deformation", 'o', "output vector field", 
			      CCmdOption::required, &C3DVFIOPluginHandler::instance()));

	options.set_group("Registration parameters"); 
	options.add(make_opt( disable_multigrid, "disable-multigrid", 0, "disable multi-grid processing"));
	options.add(make_opt( disable_fullres, "disable-fullres", 0,
			      "disable processing on the full resolution image"));
	options.add(make_opt( params.Lambda,"lambda", 0, "elasticy constant"));
	options.add(make_opt( params.My,"mu", 0, "elasticy constant"));
	options.add(make_opt( STARTSIZE, "start-size", 's', "initial multigrided size" ));
	options.add(make_opt( method, g_method_dict, "method", 'm',  "method for solving PDE"));
	options.add(make_opt( params.InitialStepsize, "step", 0, "Initial stepsize"));
	options.add(make_opt( interpolator_kernel, "bspline:d=3", "interpolator", 'p', "image interpolator kernel"));
	options.add(make_opt( params.Overrelaxation, "relax", 0, "overrelaxation factor vor method sor"));
	options.add(make_opt( params.maxiter, "maxiter", 0, "maxium iterations"));
	options.add(make_opt( params.factor, "epsilon", 0, "truncation condition"));
	options.add(make_opt( statlog_filename, "statlog", 0,"statistics logfilename"));
	options.add(make_opt( params.matter_threshold, "matter", 0, "intensity above which real "
			      "matter is assumed (experimental)"));
//	options.add(make_opt( max_threads, "max-threads", 't', "maximal number of threads for sorap"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	params.source = load_image<P3DImage>(in_filename);
	params.reference = load_image<P3DImage>(ref_filename);

	switch (method) {
	case meth_sor:solver = new TSORSolver(params.maxiter,params.factor,params.factor,params.My,params.Lambda);break;
	case meth_sorex:solver = new  TSORASolver(params.maxiter,params.factor,params.factor,params.My,params.Lambda);break;
	default:cverr() << "Unknown solver specified"<< endl; return -1;
	}

	C3DInterpolatorFactory ipf(interpolator_kernel, "mirror");

	std::shared_ptr<TLinEqnSolver > ensure_solver_delete(solver);

	g_start = Clock.get_seconds();

	P3DFVectorfield result = fluid_transform(params,solver,!disable_multigrid,
						 !disable_fullres,&g_MeasureList, ipf);


	C3DIOVectorfield outfield(result->get_size());
	copy(result->begin(), result->end(), outfield.begin());

	if (!C3DVFIOPluginHandler::instance().save(out_filename, outfield)){
		cerr << "Unable to save result vector field to " << out_filename << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
