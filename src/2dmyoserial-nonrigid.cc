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
  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dmyoserial-nonrigid}
  \label{mia-2dmyoserial-nonrigid}

  \begin{description} 
  \item [Description:] 
	This program runs the non-rigid registration of an perfusion image series. 
	The registration is run in a serial manner, this is, only images in 
	temporal succession are registered, and the obtained transformations 
	are applied accumulated to reach full registration (cf. \citet{wollny10a}). 
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyoserial-nonrigid -i <input set> -o <output set> <cost1> [<cost2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
				 
  \cmdgroup{Image registration} 
  \cmdopt{ref}{r}{int}{Reference frame to base the registration on}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{transForm}{f}{string}{Transformation space as provided by the 
                                \hyperref[sec:2dtransforms]{transformation plug-ins.}}
  }
  The cost functions are given as free parameters on the command line. 
  For available options see sections \ref{sec:2dfullcost} and \ref{sec:cost2d}. 

  \item [Example:]Register the perfusion series given in segment.set by optimizing a spline based 
                  transformation with a coefficient rate of 16 pixel 
                 using  a weighted combination of \emph{normalized gradient fields} 
                  and SSD as cost measure, and penalize the transformation by using divcurl with aweight of 2.0. 
  \begin{lstlisting}
mia-2dmyoserial-nonrigid  -i segment.set -o registered.set -F spline:rate=16 \
                     image:cost=[ngf:eval=ds],weight=2.0 image:cost=ssd,weight=0.1 divcurl:weight=2.0 
  \end{lstlisting}
  \item [See also:] \sa{mia-2dmyomilles}, \sa{mia-2dmyoperiodic-nonrigid}, 
                    \sa{mia-2dmyoica-nonrigid}, \sa{mia-2dmyopgt-nonrigid},
		    \sa{mia-2dsegseriesstats}
  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "2dmyoserial"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 


const SProgramDescrption g_general_help = {
	// .g_program_group =  
	"Myocardial Perfusion Analysis", 
	
	// .g_general_help = 
	"This program runs the non-rigid motion compensation registration of an perfusion image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession are registered, and the obtained transformations "
	"are applied accumulated to reach full registration.\n", 
	
	//.g_program_example_descr = 
	"Register the perfusion series given in 'segment.set' to reference image 30. " 
        "Skip two images at the beginning and using mutual information as cost function, "
	"and penalize the transformation by divcurl with weight 5. "
	"Store the result in 'registered.set'.\n", 
	
	//.g_program_example_code = 
	"  -i segment.set -o registered.set -k 2  -r 30 mi divcurl:weight=5"
}; 


int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");
	                        
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("spline"); 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	auto interpolator_kernel = produce_spline_kernel("bspline:d=3");
	size_t mg_levels = 3; 
	int reference_param = -1; 
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'R', 
				    "file name base for registered fiels")); 
	
	
	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 

	if (options.parse(argc, argv, "cost") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	C2DFullCostList costs; 
	for (auto c = cost_functions.begin(); c != cost_functions.end(); ++c) {
		auto cost = C2DFullCostPluginHandler::instance().produce(*c); 
		costs.push(cost); 
	}

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images.size() / 2 : reference_param; 


	// prepare registration framework 
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images.size() - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	
	// run forward registrations 
	for (size_t i = 0; i < reference; ++i) {
		P2DTransformation transform = nrr.run(input_images[i], input_images[i+1]);
		for (size_t j = 0; j <=i ; ++j) {
			input_images[j] = (*transform)(*input_images[j]);
			frames[j].inv_transform(*transform); 
		}
	}
	
	// run backward registration 
	for (size_t i = input_images.size() - 1; i > reference; --i) {
		P2DTransformation transform = nrr.run(input_images[i], input_images[i-1]);
		for (size_t j = input_images.size() - 1; j >= i ; --j) {
			input_images[j] = (*transform)(*input_images[j]);
			frames[j].inv_transform(*transform); 
		}
	}
	
	
	// prepare output set and save images 
	input_set.set_images(input_images); 
	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

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


