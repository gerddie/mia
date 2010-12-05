
/*
** Copyrigh (C) 1999 Max-Planck-Institute of Cognitive Neurosience
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

  As an exception to this license, "NEC C&C Research Labs" may use
  this software under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation.


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

#include "vfluid.hh"
#include "sor_solver.hh"


using namespace mia;
using namespace std;

/**
 * \page miafluid3d miafluid3d a program for non rigid registration
 *
 *
 * This program is used for non-rigid registration based on fluid dynamics.
 * It is called like this:

     miafluid3d <options>

   Where options include but are not limited to

   \param -in  The input image to be registered \a required
   \param -ref The reference image to be registered to \a required
   \param -out The output vector field describing the deformation in an reverse Eulerien reference frame \a required

   \param -method Method for solving the underlying PDE including
   \li \em sor - successive overrelaxation
   \li \em sora - successive overrelaxation with adaptive update
   \li \em soarp - successive overrelaxation with adaptive update, parallized
   \li \em cg - conjugated gradients (needs lots of ram)
   \li \em conv - convolution filters    (default = sor)

   \param -maxiter maximum number of iterations in solving the PDE (default = 20)
   \param -epsilon relative error to stop iterations in solving the PDE (default = 0.01)
   \param -startsize estimate for start size in multi-grid processing (default = 32)
   \param -max-threads maximum number of threads in parallized run (default = 2)
   \param -halffilterdim half size if the filter when using \a conv (default = 3)
   \param -statlog a file to write out some statistics
   \param -mu Lamé parameter - modulus of volume expansion (default = 1.0)
   \param -lambda Lamé parameter  - modulus of elasticy in shear (default = 1.0)

 * \author Gert Wollny <wollny@cns.mpg.de>
*/




#define MU	  1	// Lame elasticity constants
#define LAMBDA	  1	//

enum TMethod { meth_sor,
	       meth_cg,
	       meth_filter,
	       meth_sorex,
	       meth_sorap
};


const TDictMap<TMethod>::Table method_dict[] = {
	{"sor", meth_sor}
	,{"sora",meth_sorex}
	,{"sorap",meth_sorap}
	,{NULL, meth_sor}
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

const char *g_description = "This code implements non-rigid image registration "
	"by implementing a fluid-dynamic transformation model."; 

int main(int argc, const char *argv[])
{
	//	FILE *srcf,*reff,*outf;

	TMethod method = meth_sorap;

	TLinEqnSolver *solver=NULL;

    	TFluidRegParams params;

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
	params.interp_type = ip_bspline3;


	//	bool in_found;
	int  max_threads = 2;

	string in_filename;
	string out_filename;
	string ref_filename;
	string statlog_filename;

	bool disable_multigrid = false;
	bool disable_fullres = false;

	cverb.set_verbosity( vstream::ml_message );

	CCmdOptionList options(g_description);
	options.push_back(make_opt( in_filename, "in-image", 'i', "input image", CCmdOption::required ));
	options.push_back(make_opt( ref_filename, "ref-image", 'r', "reference image ", CCmdOption::required ));
	options.push_back(make_opt( out_filename, "out-deformation", 'o', "output vector field", CCmdOption::required ));
	options.push_back(make_opt( disable_multigrid, "disable-multigrid", 0, "disable multi-grid processing"));
	options.push_back(make_opt( disable_fullres, "disable-fullres", 0,
				    "disable processing on the full resolution image"));
	options.push_back(make_opt( params.Lambda,"lambda", 0, "elasticy constant"));
	options.push_back(make_opt( params.My,"mu", 0, "elasticy constant"));
	options.push_back(make_opt( STARTSIZE, "start-size", 's', "initial multigrided size" ));
	options.push_back(make_opt( method, g_method_dict, "method", 'm',  "method for solving PDE"));
	options.push_back(make_opt( params.InitialStepsize, "step", 0, "Initial stepsize"));
	options.push_back(make_opt( params.interp_type, GInterpolatorTable, "interpolator", 'p', 
				    "image transformation interpolator"));
	options.push_back(make_opt( params.Overrelaxation, "relax", 0, "overrelaxation factor vor method sor"));
	options.push_back(make_opt( params.maxiter, "maxiter", 0, "maxium iterations in sor and cg"));
	options.push_back(make_opt( params.factor, "epsilon", 0, "truncation condition in sor and cg"));
	options.push_back(make_opt( statlog_filename, "statlog", 0,"statistics logfilename"));
	options.push_back(make_opt( params.matter_threshold, "matter", 0, "intensity above which real "
				    "matter is assumed (experimental)"));
	options.push_back(make_opt( max_threads, "max-threads", 't', "maximal number of threads for sorap"));

	try {
		options.parse(argc, argv);

		params.source = load_image3d(in_filename);
		params.reference = load_image3d(ref_filename);

		switch (method) {
		case meth_sor:solver = new TSORSolver(params.maxiter,params.factor,params.factor,params.My,params.Lambda);break;
		case meth_sorex:solver = new  TSORASolver(params.maxiter,params.factor,params.factor,params.My,params.Lambda);break;
		case meth_sorap:cvwarn() << "Parallel solver not tested with new thread model\n";
			solver = new  TSORAParallelSolver(params.maxiter,params.factor,params.factor,params.My,params.Lambda,max_threads);break;
		default:cverr() << "Unknown solver specified"<< endl; return -1;
		}

		std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(params.interp_type));

		std::shared_ptr<TLinEqnSolver > ensure_solver_delete(solver);

		g_start = Clock.get_seconds();

		P3DFVectorfield result = fluid_transform(params,solver,!disable_multigrid,
							 !disable_fullres,&g_MeasureList, *ipf);


		C3DIOVectorfield outfield(result->get_size());
		copy(result->begin(), result->end(), outfield.begin());

		if (!C3DVFIOPluginHandler::instance().save("", out_filename, outfield)){
			cerr << "Unable to save result vector field to " << out_filename << "\n";
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	catch (runtime_error& error) {
		cverr() << error.what() << endl;
	}

	catch (invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;

}

