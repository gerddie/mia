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
  LatexBeginProgramDescription{3D image processing}

  \subsection{mia-3dtransform}
  \label{mia-3dtransform}
  
  \begin{description}
  \item [Description:] Transform a 3D image by applying a given 3D transformation.  
  The program is called like 
  \
  \begin{lstlisting}
mia-3dtransform -i <input> -t <transformaton> -o <output>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \cmdopt{transformation}{t}{string}{file name of the transformation}
  \cmdopt{interpolator}{p}{string}{image interpolation kernel to use}
  }

  \item [Example:] Transform an image input.v by the transfromation stored in trans.v 
        by using nearest neighbour interpolation ans store the result in output.v 
   \
  \begin{lstlisting}
mia-3dtransform -i input.v -t trans.v  -o output.v  -p nn 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/


#define VSTREAM_DOMAIN "mia-3dtransform"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/3dimageio.hh>


static const char *program_info = 
	"This program is used to transform an 3D image by using a given generic transformation.\n"
	"Basic usage:\n"
	"  mia-3dimagefilter -i <input> -t <transformation> -o <output> \n"; 



NS_MIA_USE
using namespace std;

int do_main(int argc, const char **argv)
{
	CCmdOptionList options(program_info);

	string src_filename;
	string out_filename;
	string trans_filename;
	EInterpolation interpolator = ip_bspline3;

	options.add(make_opt( src_filename, "in-file", 'i', "input image", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "reference image", CCmdOption::required));
	options.add(make_opt( trans_filename, "transformation", 't', "transformation file", 
				    CCmdOption::required));
	options.add(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p', "image interpolator"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	auto transformation = C3DTransformationIOPluginHandler::instance().load(trans_filename);

	auto source = imageio.load(src_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no vector field found in " << trans_filename << "\n";
		return EXIT_FAILURE;
	}

	std::shared_ptr<C3DInterpolatorFactory > ipf(create_3dinterpolation_factory(interpolator));

	for (auto i = source->begin(); i != source->end(); ++i)
		*i = (*transformation)(**i, *ipf);

	if ( !imageio.save("", out_filename, *source) ){
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
