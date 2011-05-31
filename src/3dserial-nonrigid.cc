/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
  LatexBeginProgramDescription{Registration of image series}
  
  \subsection{mia-3dserial-nonrigid}
  \label{mia-3dserial-nonrigid}

  \begin{description} 
  \item [Description:] 
	This program runs the non-rigid registration of an perfusion image series. 
	The registration is run in a serial manner, this is, only images in 
	temporal succession are registered, and the obtained transformations 
	are applied accumulated to reach full registration (cf. \citet{wollny10a}). 
  
  The program is called like 
  \begin{lstlisting}
mia-3dserial-nonrigid -i <input set> -o <output set> <cost1> [<cost2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input images}
  \cmdopt{out-file}{o}{string}{output images}
				 
  \cmdgroup{Image registration} 
  \cmdopt{ref}{r}{int}{Reference frame to base the registration on}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{transForm}{f}{string}{Transformation space as provided by the 
                                \hyperref[sec:3dtransforms]{transformation plug-ins.}}
  }
  The cost functions are given as free parameters on the command line. 
  For available options see sections \ref{sec:3dfullcost} and \ref{sec:cost3d}. 

  \item [Example:]Register the perfusion series given in segment.set by optimizing a spline based 
                  transformation with a coefficient rate of 16 pixel 
                 using  a weighted combination of \emph{normalized gradient fields} 
                  and SSD as cost measure, and penalize the transformation by using divcurl with aweight of 2.0. 
  \begin{lstlisting}
mia-3dserial-nonrigid  -i segment.set -o registered.set -F spline:rate=16 \
                     image:cost=[ngf:eval=ds],weight=2.0 image:cost=ssd,weight=0.1 divcurl:weight=2.0 
  \end{lstlisting}
  \item [See also:] \sa{mia-3dmyomilles}, \sa{mia-3dmyoperiodic-nonrigid}, 
                    \sa{mia-3dmyoica-nonrigid}, \sa{mia-3dmyopgt-nonrigid},
		    \sa{mia-3dsegseriesstats}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "3dmyoserial"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/filetools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/3dimageio.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program runs the non-rigid registration of an perfusion image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession are registered, and the obtained transformations "
	"are applied accumulated to reach full registration. "
  	"Basic usage: \n"
	" mia-3dserial-nonrigid [options] <cost1> <cost2> ..."; 


int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg%04d.v");
	                        
	auto transform_creator = C3DTransformCreatorHandler::instance().produce("spline"); 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	EInterpolation interpolator = ip_bspline3;
	size_t mg_levels = 3; 
	int reference_param = -1; 
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', 
				    "file name for registered fiels")); 
	
	
	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	C3DFullCostList costs; 
	for (auto c = cost_functions.begin(); c != cost_functions.end(); ++c) {
		auto cost = C3DFullCostPluginHandler::instance().produce(*c); 
		costs.push(cost); 
	}
	
	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(interpolator));

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	C3DImageSeries input_images; 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images.push_back(image);
	}

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images.size() / 2 : reference_param; 

	// prepare registration framework 
	C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, *ipfactory, mg_levels);
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images.size() - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	
	// run forward registrations 
	for (size_t i = 0; i < reference; ++i) {
		P3DTransformation transform = nrr.run(input_images[i], input_images[i+1]);
		for (size_t j = 0; j <=i ; ++j) {
			input_images[j] = (*transform)(*input_images[j], *ipfactory);
		}
	}
	
	// run backward registration 
	for (size_t i = input_images.size() - 1; i > reference; --i) {
		P3DTransformation transform = nrr.run(input_images[i], input_images[i-1]);
		for (size_t j = input_images.size() - 1; j >= i ; --j) {
			input_images[j] = (*transform)(*input_images[j], *ipfactory);
		}
	}
	

	bool success = true; 
	auto ii = input_images.begin(); 
	for (size_t i = start_filenum; i < end_filenum; ++i, ++ii) {
		string out_name = create_filename(registered_filebase.c_str(), i);
		cvmsg() << "Save image " << i << " to " << out_name << "\n"; 
		success &= save_image(out_name, *ii); 
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
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


