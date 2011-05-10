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
  LatexBeginProgramSection{3D image registration}
  \label{sec:3dreg}
  
  These programs run provide types of 3D image registration. 

  LatexEnd
*/


/*
  LatexBeginProgramDescription{3D image registration}
  
  \subsection{mia-3drigidreg}
  \label{mia-3drigidreg}

  \begin{description}
  \item [Description:] This program implements the registration of two gray scale 3D images. 
	The transformation is not penalized, therefore, one should only use translation, rigid, or affine 
	transformations as target and run mia-3dnonrigidreg of nonrigid registration is to be achieved.

  The program is called like 
  \
  \begin{lstlisting}
mia-3drigidreg -i <input image> -r <reference image> -o <output image> [options]
  \end{lstlisting}
  

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optreffile
  \optoutfile
  \cmdopt{cost}{c}{string}{Cost function as given in section \ref{sec:cost3d}  }
  \cmdopt{levels}{l}{int}{multiresolution processing levels }
  \cmdopt{optimizer}{O}{string}{optimizer as given in section \ref{sec:minimizers}  }
  \cmdopt{trans}{t}{string}{transformation output file  }
  \cmdopt{transForm}{f }{string}{transformation type to achieve registration as given in section \ref{sec:3dtransforms} 
     }
  }

  \item [Example:]Register image test.v to image ref.v affine and write the registered image to reg.v. 
  Use two multiresolution levels and ssd as cost function. 
   \
  \begin{lstlisting}
mia-3drigidreg -i test.v -r ref.v -o reg.v -l 2 -f affine -c ssd 
  \end{lstlisting}
  \item [Remark:] The implementation allows to use a non-linear transformation, like \emph{spline} as target 
    transformation, but this is not advisable. 
  \end{description}
  LatexEnd
*/



#include <sstream>
#include <mia/core.hh>
#include <mia/core/minimizer.hh>
#include <mia/3d.hh>
#include <mia/3d/rigidregister.hh>
#include <mia/3d/transformio.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE;
using namespace std;

const char *g_description = 
	"This program implements the registration of two gray scale 3D images. "
	"The transformation is not penalized, therefore, one should only use translation, rigid, or affine "
	"transformations as target and run mia-3dnonrigidreg of nonrigid registration is to be achieved." 
	; 

int do_main( int argc, const char *argv[] )
{
	string cost_function("ssd"); 
	string src_filename;
	string ref_filename;
	string out_filename;
	string trans_filename;
	auto transform_creator = C3DTransformCreatorHandler::instance().produce("rigid"); 
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=simplex,step=1.0"); 

	size_t mg_levels = 3;

	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_filename, "in", 'i', "test image", CCmdOption::required));
	options.push_back(make_opt( ref_filename, "ref", 'r', "reference image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out", 'o', "registered output image", CCmdOption::required));
	options.push_back(make_opt( trans_filename, "trans", 't', "transformation output file name"));
	options.push_back(make_opt( cost_function, "cost", 'c', "cost function")); 
	options.push_back(make_opt( mg_levels, "levels", 'l', "multigrid levels"));
	options.push_back(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.push_back(make_opt( transform_creator, "transForm", 'f', "transformation type"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	P3DImage Model = load_image<P3DImage>(src_filename);
	P3DImage Reference = load_image<P3DImage>(ref_filename);

	C3DBounds GlobalSize = Model->get_size();
	if (GlobalSize != Reference->get_size()){
		throw std::invalid_argument("Images have different size");
	}

	auto cost = C3DImageCostPluginHandler::instance().produce(cost_function);
	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(ip_bspline3));

	C3DRigidRegister rr(cost, minimizer,  transform_creator, *ipfactory, mg_levels);

	P3DTransformation transform = rr.run(Model, Reference);
	P3DImage result = (*transform)(*Model, *ipfactory);

	if (!trans_filename.empty()) {
		if (!C3DTransformationIOPluginHandler::instance().save("", trans_filename, *transform)) 
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
