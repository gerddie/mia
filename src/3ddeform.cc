/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
  LatexBeginProgramDescription{3D image processing}

  \subsection{mia-3ddeform}
  \label{mia-3ddeform}
  
  \begin{description}
  \item [Description:] Transform a 3D image by applying a given 3D transformation that is defined 
  by a 3D vector field $\vu$ like $\vx \rightarrow \vx - \vu(\vx)$. 
  The program is called like 
  \
  \begin{lstlisting}
mia-3ddeform -i <input> -t <transformaton> -o <output>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \cmdopt{transformation}{t}{string}{file name of the transformation}
  \cmdopt{interpolator}{p}{string}{image interpolation kernel to use}
  }

  \item [Example:] Transform an image input.v by the transformation defined by the vector field field.v 
        by using bspline interpolation of degree 4 and store the result in output.v 
   \
  \begin{lstlisting}
mia-3dtransform -i input.v -t field.v  -o output.v  -p bspline4 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/


#include <mia/core.hh>
#include <mia/3d.hh>

#include <mia/3d/deformer.hh>

static const char *program_info = 
	"This program is used to transform an 3D image by using a given transformation.\n"
	"Basic usage:\n"
	"  mia-3dimagefilter -i <input> -t <transformation> -o <output> [<plugin>]\n"; 


NS_MIA_USE
using namespace boost;
using namespace std;

typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;


int do_main(int argc, const char **argv)
{
	CCmdOptionList options(program_info);

	string src_filename;
	string out_filename;
	string vf_filename;
	auto interpolator_kernel = produce_spline_kernel("bspline:d=3");

	options.add(make_opt( src_filename, "in-file", 'i', "input image", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "reference image", CCmdOption::required));
	options.add(make_opt( vf_filename, "transformation", 't', "transformation vector field", 
				    CCmdOption::required));
	options.add(make_opt( interpolator_kernel ,"interpolator", 'p', "image interpolator kernel"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	const C3DVFIOPluginHandler::Instance&  vfioh = C3DVFIOPluginHandler::instance();

	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;
	typedef C3DVFIOPluginHandler::Instance::PData P3DVF;
	PImageVector source    = imageio.load(src_filename);
	P3DVF transformation   = vfioh.load(vf_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no vector field found in " << vf_filename << "\n";
		return EXIT_FAILURE;
	}

	P3DInterpolatorFactory ipfactory(new C3DInterpolatorFactory(interpolator_kernel, 
								    PBoundaryCondition(new CMirrorOnBoundary), 
								    PBoundaryCondition(new CMirrorOnBoundary), 
								    PBoundaryCondition(new CMirrorOnBoundary)));

	FDeformer3D deformer(*transformation,*ipfactory);


	for (C3DImageIOPluginHandler::Instance::Data::iterator i = source->begin();
	     i != source->end(); ++i)
		*i = filter(deformer, **i);

	if ( !imageio.save(out_filename, *source) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};
	return EXIT_SUCCESS;
}


int main(int argc, const char **argv)
{
	try {
		return do_main(argc, argv);
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n";
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n";
	}
	catch (std::exception& err) {
		cerr << "exception: " << err.what() << "\n";
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE;
}
