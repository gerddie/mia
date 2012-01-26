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

/*
  LatexBeginProgramDescription{3D image registration}

  \subsection{mia-3dnonrigidreg}
  \label{mia-3dnonrigidreg}
  
  \begin{description}
  \item [Description:] This program implements the non-linear registration of two gray scale 3D images. 

  The program is called like 
  \
  \begin{lstlisting}
mia-3dnonrigidreg -i <input image> -r <reference image> -o <output image> \
               [options] <cost1> [<cost2>] ...
  \end{lstlisting}
  

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optreffile
  \optoutfile
  \cmdopt{levels}{l}{int}{multiresolution processing levels}
  \cmdopt{optimizer}{O}{string}{optimizer as given in section \ref{sec:minimizers} }
  \cmdopt{trans}{t}{string}{transformation output file}
  \cmdopt{transForm}{f }{string}{transformation type to achieve registration as given 
          in section \ref{sec:3dtransforms}}
  }

  The cost functions are given as extra parameters on the command line. 
  These may include any combination of the cost functions given in section \ref{sec:3dfullcost}. 


  \item [Example:]Register image test.v to image ref.v by using a spline transformation with a 
  coefficient rate of 5  and write the registered image to reg.v. 
  Use two multiresolution levels, ssd as image cost function and divcurl weighted by 10.0 
   as transformation smoothness penalty. 
   \
  \begin{lstlisting}
mia-3dnonrigidreg -i test.v -r ref.v -o reg.v -l 2 \
                  -f spline:rate=3  image:cost=ssd divcurl:weight=10
  \end{lstlisting}
 
  \item [Remark:] The implementation allows to use a linear transformation, like \emph{rigid} as target 
    transformation supersetting mia-3drigidreg. 
    Of course giving a transformation penalty wouldn't make sense in such cases.
  \end{description}
  LatexEnd
*/

#include <sstream>
#include <tbb/task_scheduler_init.h>

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
	"3D image registration", 
	
	"This program implements the registration of two gray scale 3D images.", 
	
	"Register image test.v to image ref.v by using a spline transformation with a "
	"coefficient rate of 5  and write the registered image to reg.v. "
	"Use two multiresolution levels, ssd as image cost function and divcurl weighted by 10.0 "
	"as transformation smoothness penalty. ",
	
	"-i test.v -r ref.v -o reg.v -l 2 -f spline:rate=3  image:cost=ssd divcurl:weight=10"
};

int do_main( int argc, char *argv[] )
{
	string cost_function("ssd"); 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	PMinimizer minimizer; 
	int max_threads = tbb::task_scheduler_init::automatic;

	cvdebug() << "auto transform_creator\n"; 
	P3DTransformationFactory transform_creator; 

	size_t mg_levels = 3;

	CCmdOptionList options(g_description);
	options.set_group("IO"); 
	options.add(make_opt( src_filename, "in", 'i', "test image", CCmdOption::required));
	options.add(make_opt( ref_filename, "ref", 'r', "reference image", CCmdOption::required));
	options.add(make_opt( out_filename, "out", 'o', "registered output image", CCmdOption::required));
	options.add(make_opt( trans_filename, "trans", 't', "output transformation"));
	
	options.set_group("Registration"); 
	options.add(make_opt( mg_levels, "levels", 'l', "multi-resolution levels"));
	options.add(make_opt( minimizer, "gsl:opt=gd,step=0.1", "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( transform_creator, "spline:rate=10", "transForm", 'f', "transformation type"));

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 'T', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  

	if (options.parse(argc, argv, "cost", &C3DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	tbb::task_scheduler_init init(max_threads);
	
	auto cost_descrs = options.get_remaining(); 

	C3DFullCostList costs; 
	for (auto i = cost_descrs.begin(); i != cost_descrs.end(); ++i)
		costs.push(C3DFullCostPluginHandler::instance().produce(*i)); 

	P3DImage Model = load_image<P3DImage>(src_filename);
	P3DImage Reference = load_image<P3DImage>(ref_filename);
	C3DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size())
		throw std::invalid_argument("Images have different size");

	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(ip_bspline3, bc_mirror_on_bounds));
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
