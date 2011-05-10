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
  LatexBeginProgramDescription{2D image processing}

  \subsection{mia-2dtransform}
  \label{mia-2dtransform}

  \begin{description}
  \item [Description:] Transform a 2D image by applying a given 2D transformation.  
  The program is called like 
  \
  \begin{lstlisting}
mia-2dtransform -i <input> -t <transformaton> -o <output>
  \end{lstlisting}
  \item [Options:] $\:$

  \tabstart
  \optinfile
  \optoutfile
  \cmdopt{transformation}{t}{string}{file name of the transformation}
  \cmdopt{interpolator}{p}{string}{image interpolation kernel to use}
  \tabend

  \item [Example:] Transform an image input.png by the transfromation stored in trans.v
        by using linear interpolation and store the result in output.png 
   \
  \begin{lstlisting}
mia-2dtransform -i input.png -t trans.v  -o output.png  -p linear
  \end{lstlisting}
  \end{description}

  LatexEnd
*/


#include <mia/core.hh>
#include <mia/2d.hh>

#include <mia/2d/transformio.hh>

NS_MIA_USE
using namespace boost;
using namespace std;


const char *g_description = 
	"This program is used to deform a 2D image using a given transformation."
;

int do_main(int argc, const char **argv)
{
	CCmdOptionList options(g_description);
	string src_filename;
	string out_filename;
	string trans_filename;
	EInterpolation interpolator = ip_bspline3;

	options.push_back(make_opt( src_filename, "in-file", 'i', "input image", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output image", CCmdOption::required));
	options.push_back(make_opt( trans_filename, "transformation", 't', "transformation file name", 
				    CCmdOption::required));
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p', "image interpolator"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	typedef C2DImageIOPluginHandler::Instance::PData PImageVector;
	typedef C2DVFIOPluginHandler::Instance::PData P3DVF;
	PImageVector source    = imageio.load(src_filename);
	auto transformation   = C2DTransformationIOPluginHandler::instance().load(trans_filename);

	if (!source || source->size() < 1) {
		cerr << "no image found in " << src_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!transformation) {
		cerr << "no transformation field found in " << trans_filename << "\n";
		return EXIT_FAILURE;
	}

	std::shared_ptr<C2DInterpolatorFactory > ipf(create_2dinterpolation_factory(interpolator));

	for (C2DImageIOPluginHandler::Instance::Data::iterator i = source->begin();
	     i != source->end(); ++i)
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
