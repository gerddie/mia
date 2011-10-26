/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
  LatexBeginProgramDescription{2D image registration}

  \subsection{mia-2dnonrigidreg}
  \label{mia-2dnonrigidreg}

  \begin{description}
  \item [Description:] This program implements the non-linear registration of two gray scale 2D images. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2drigidreg -i <input image> -r <reference image> -o <output image> \
               [options] <cost1> [<cost2>] ...
  \end{lstlisting}
  

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optreffile
  \optoutfile
  \cmdopt{levels}{l}{int}{multiresolution processing levels }
  \cmdopt{optimizer}{O}{string}{optimizer as given in section \ref{sec:minimizers}  }
  \cmdopt{trans}{t}{string}{transformation output file}
  \cmdopt{transForm}{f }{string}{transformation type to achieve registration as given 
                                 in section \ref{sec:2dtransforms}}
  }

  The cost functions are given as extra parameters on the command line. 
  These may include any combination of the cost functions given in section \ref{sec:2dfullcost}. 


  \item [Example:]Register image test.v to image ref.v by using a spline transformation with a 
  coefficient rate of 5  and write the registered image to reg.v. 
  Use two multiresolution levels, ssd as image cost function and divcurl weighted by 10.0 
   as transformation smoothness penalty. 
   \
  \begin{lstlisting}
mia-2dnonrigidreg -i test.v -r ref.v -o reg.v -l 2 \
                  -f spline:rate=3  image:cost=ssd divcurl:weight=10
  \end{lstlisting}
 
  \item [Remark:] The implementation allows to use a linear transformation, like \emph{rigid} as target 
    transformation supersetting mia-2drigidreg. 
    Of course giving a transformation penalty wouldn't make sense in such cases.
  \end{description}
  LatexEnd
*/


#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescrption g_general_help = {
	"Image Registration", 
	
	"This program runs registration of two images optimizing a transformation of the given transformation model "
	"by optimizing certain cost measures.\n", 
	
	"Register the image 'moving.png' to the image 'reference.png' by using a rigid transformation model "
        " and ssd as cost function. Write the result to output.png", 
	
	"  -i moving.png -r reference.png -o output.png -f rigid image:cost=ssd"
}; 

int do_main( int argc, char *argv[] )
{
	string cost_function("ssd"); 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	string transform_type("spline");
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	PMinimizer refinement_minimizer;
	cvdebug() << "auto transform_creator\n"; 
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("spline"); 
	if (!transform_creator && transform_creator->get_init_string() != string("spline"))
		cverr() << "something's wrong\n"; 

	size_t mg_levels = 3;

	CCmdOptionList options(g_general_help);
	options.add(make_opt( src_filename, "in", 'i', "test image", CCmdOption::required));
	options.add(make_opt( ref_filename, "ref", 'r', "reference image", CCmdOption::required));
	options.add(make_opt( out_filename, "out", 'o', "registered output image", CCmdOption::required));
	options.add(make_opt( trans_filename, "trans", 't', "output transformation"));
	options.add(make_opt( mg_levels, "levels", 'l', "multi-resolution levels"));
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( refinement_minimizer, "refiner", 'R',
			      "optimizer used for refinement after the main optimizer was called"));
	
	options.add(make_opt( transform_creator, "transForm", 'f', "transformation type"));

	if (options.parse(argc, argv, "cost") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
	auto cost_descrs = options.get_remaining(); 

	C2DFullCostList costs; 
	for (auto i = cost_descrs.begin(); i != cost_descrs.end(); ++i)
		costs.push(C2DFullCostPluginHandler::instance().produce(*i)); 

	P2DImage Model = load_image<P2DImage>(src_filename);
	P2DImage Reference = load_image<P2DImage>(ref_filename);
	C2DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size())
		throw std::invalid_argument("Images have different size");

	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	if (refinement_minimizer)
		nrr.set_refinement_minimizer(refinement_minimizer); 
	
	P2DTransformation transform = nrr.run(Model, Reference);
	P2DImage result = (*transform)(*Model);

	if (!trans_filename.empty()) {
		if (!C2DTransformationIOPluginHandler::instance().save(trans_filename, *transform)) 
			cverr() << "Saving the transformation to '" << trans_filename << "' failed."; 
	}

	return save_image(out_filename, result);
}

MIA_MAIN(do_main); 
