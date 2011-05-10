/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/*
  LatexBeginProgramSection{2D image registration}
  \label{sec:2dreg}
  
  These programs run provide types of 2D image registration. 

  LatexEnd
*/


/*
  LatexBeginProgramDescription{2D image registration}

  \subsection{mia-2drigidreg}
  \label{mia-2drigidreg}
  
  \begin{description}
  \item [Description:] This program implements the registration of two gray scale 2D images. 
	The transformation is not penalized, therefore, one should only use translation, rigid, or affine 
	transformations as target and run mia-2dnonrigidreg of nonrigid registration is to be achieved.

  The program is called like 
  \
  \begin{lstlisting}
mia-2drigidreg -i <input image> -r <reference image> -o <output image> [options]
  \end{lstlisting}
  

  \item [Options:] $\:$

  \tabstart
  \optinfile
  \optreffile
  \optoutfile
  \cmdopt{cost }{c}{string}{Cost function as given in section \ref{sec:cost2d}  }
  \cmdopt{levels}{l}{int}{multiresolution processing levels }
  \cmdopt{optimizer}{O}{string}{optimizer as given in section \ref{sec:minimizers}  }
  \cmdopt{trans}{t}{string}{transformation output file  }
  \cmdopt{transForm}{f}{string}{transformation type to achieve registration as given in 
            section \ref{sec:2dtransforms}  }
  \tabend

  \item [Example:]Register image test.v to image ref.v affine and write the registered image to reg.v. 
  Use two multiresolution levels and ssd as cost function. 
   \
  \begin{lstlisting}
mia-2drigidreg -i test.v -r ref.v -o reg.v -l 2 -f affine -c ssd 
  \end{lstlisting}
  \item [Remark:] The implementation allows to use a non-linear transformation, like \emph{spline} as target 
    transformation, but this is not advisable. 
  \end{description}
  LatexEnd
*/


#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/rigidregister.hh>
#include <mia/core/minimizer.hh>
#include <mia/2d/transformio.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE;
using namespace std;

const char *g_description = 
	"This program implements the registration of two gray scale 2D images. "
	"The transformation is not penalized, therefore, one should only use translation, rigid, or affine "
	"transformations as target and run mia-2dnonrigidreg of nonrigid registration is to be achieved." 
	; 

int do_main( int argc, const char *argv[] )
{
	string cost_function("ssd"); 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("rigid"); 
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=simplex,step=1.0"); 

	size_t mg_levels = 3;

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in", 'i', "test image", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref", 'r', "reference image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out", 'o', "registered output image", CCmdOption::required));
	options.push_back(make_opt( trans_filename, "trans", 't', "transformation"));
	options.push_back(make_opt( cost_function, "cost", 'c', "cost function")); 
	options.push_back(make_opt( mg_levels, "levels", 'l', "multigrid levels"));
	options.push_back(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.push_back(make_opt( transform_creator, "transForm", 'f', "transformation type"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	P2DImage Model = load_image<P2DImage>(src_filename);
	P2DImage Reference = load_image<P2DImage>(ref_filename);

	C2DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size()){
		throw std::invalid_argument("Images have different size");
	}

	auto cost = C2DImageCostPluginHandler::instance().produce(cost_function);
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));

	C2DRigidRegister rr(cost, minimizer,  transform_creator, *ipfactory, mg_levels);

	P2DTransformation transform = rr.run(Model, Reference);
	P2DImage result = (*transform)(*Model, *ipfactory);

	if (!trans_filename.empty()) {
		if (!C2DTransformationIOPluginHandler::instance().save("", trans_filename, *transform)) 
			cverr() << "Saving the transformation to '" << trans_filename << "' failed."; 
	}

	return save_image(out_filename, result);
}


int main( int argc, const char *argv[] )
{
	try {
		return do_main(argc, argv);
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}
	return EXIT_FAILURE;
}
