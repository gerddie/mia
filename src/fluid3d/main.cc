/* -*- mia-c++  -*-
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
*/

/*
  LatexBeginProgramDescription{3D image registration}
  
  \subsection{mia-fluid3d}
  \label{mia-fluid3d}
  
  \begin{description}
  \item [Description:] This program is used for non-rigid registration based on fluid dynamics.
  It uses SSD as the sole registration criterion. For details please see \cite{wollny02comput}.

  The program is called like 
  \lstset{language=bash}
  \begin{lstlisting}
mia-fluid3d -i <input image> -r <reference image> -o <output image> [options]
  \end{lstlisting}
  

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optreffile
  \cmdopt{out-file}{o}{string}{output file to store the registering transformation}
  \cmdopt{def-file}{d }{string}{transformation output file}
  \cmdopt{epsilon}{}{float}{threshhold to stop the registration at a multi-grid level }
  \cmdopt{interpolator}{p}{string}{image interpolator
           (bspline2|bspline3|bspline4|bspline5|nn|omoms3|tri) }
  \cmdopt{lambda}{}{float}{elasticity constant}
  \cmdopt{maxiter}{}{int}{maxiumum number of iterations to solve the PDE  }
  \cmdopt{max-threads}{t}{int}{number of threads to use}
  \cmdopt{method}{m}{string}{solver method to be used for the PDE}
  \cmdopt{mu}{}{float}{elasticity constant}
  \cmdopt{relax}{}{float}{relaxation factor for PDE solver }
  \cmdopt{start-size}{s}{int}{multiresolution start size }
  \cmdopt{step}{}{float}{initial step size}
  }

  \item [Example:]Register image test.v to image ref.v and write the deformation vector field regfield.v. 
  Start registration at the smallest size above 16 pixel. 
   \lstset{language=bash}
  \begin{lstlisting}
mia-fluid3d -i test.v -r ref.v -o regfield.v -s 16 
  \end{lstlisting}
  \end{description}
  LatexEnd
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
	options.add(make_opt( in_filename, "in-image", 'i', "input image", CCmdOption::required ));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image ", CCmdOption::required ));
	options.add(make_opt( out_filename, "out-deformation", 'o', "output vector field", CCmdOption::required ));
	options.add(make_opt( disable_multigrid, "disable-multigrid", 0, "disable multi-grid processing"));
	options.add(make_opt( disable_fullres, "disable-fullres", 0,
				    "disable processing on the full resolution image"));
	options.add(make_opt( params.Lambda,"lambda", 0, "elasticy constant"));
	options.add(make_opt( params.My,"mu", 0, "elasticy constant"));
	options.add(make_opt( STARTSIZE, "start-size", 's', "initial multigrided size" ));
	options.add(make_opt( method, g_method_dict, "method", 'm',  "method for solving PDE"));
	options.add(make_opt( params.InitialStepsize, "step", 0, "Initial stepsize"));
	options.add(make_opt( params.interp_type, GInterpolatorTable, "interpolator", 'p', 
				    "image transformation interpolator"));
	options.add(make_opt( params.Overrelaxation, "relax", 0, "overrelaxation factor vor method sor"));
	options.add(make_opt( params.maxiter, "maxiter", 0, "maxium iterations in sor and cg"));
	options.add(make_opt( params.factor, "epsilon", 0, "truncation condition in sor and cg"));
	options.add(make_opt( statlog_filename, "statlog", 0,"statistics logfilename"));
	options.add(make_opt( params.matter_threshold, "matter", 0, "intensity above which real "
				    "matter is assumed (experimental)"));
	options.add(make_opt( max_threads, "max-threads", 't', "maximal number of threads for sorap"));

	try {
		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 


		params.source = load_image<P3DImage>(in_filename);
		params.reference = load_image<P3DImage>(ref_filename);

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

