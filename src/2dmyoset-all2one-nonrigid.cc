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
  LatexBeginProgramDescription{2D registration of series of images}
  
  \subsection{mia-2dmyoset-all2one-nonrigid}
  \label{mia-2dmyoset-all2one-nonrigid}

  \begin{description} 
  \item [Description:] 
  This program runs the non-rigid registration of an image series. 
  All images are registered to one refernces as given on the command line. 
  If no reference is given then the image in the middle of the series is selected. 
  Registration can be run in parallel.
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyoset-all2one-nonrigid -i <input set> -o <output set> <cost1> [<cost2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output  segmentation set}
				 
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
		  using  \emph{mutual information} 
                  and penalize the transformation by using divcurl with aweight of 2.0. 
  \begin{lstlisting}
mia-2dmyoset-all2one-nonrigid  -i segment.set -o registered.set -F spline:rate=16 \
                     image:cost=mi,weight=2.0 divcurl:weight=2.0 
  \end{lstlisting}
  \item [See also:] \sa{mia-2dserial-nonrigid}, \sa{mia-2dmyoica-nonrigid}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "2dall2one"

#include <fstream>
#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp>

#include <mia/core.hh>
#include <mia/core/threadedmsg.hh>

#include <mia/internal/main.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/SegSetWithImages.hh>


#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

using namespace tbb;
using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescrption g_description = {
	"Myocardial Perfusion Analysis", 

	"This program runs the non-rigid registration of an perfusion image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession are registered, and the obtained transformations "
	"are applied accumulated to reach full registration.", 

	"Register the perfusion series given in segment.set by optimizing a spline based "
	"transformation with a coefficient rate of 16 pixel "
	"using Mutual Information and penalize the transformation by using divcurl with aweight of 2.0.", 

	"-i segment.set -o registered.set -F spline:rate=16 image:cost=mi,weight=2.0 divcurl:weight=2.0"
}; 

C2DFullCostList create_costs(const std::vector<const char *>& costs, int idx)
{
	stringstream cost_descr; 
	cost_descr << ",src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	C2DFullCostList result; 

	for (auto c = costs.begin(); c != costs.end(); ++c) {
		string cc(*c); 

		if (cc.find("image") == 0) 
			cc.append(cost_descr.str()); 
		cvdebug() << "create cost:"  << *c << " as " << cc << "\n"; 
		auto imagecost = C2DFullCostPluginHandler::instance().produce(cc);
		result.push(imagecost); 
	}

	return result; 
}

struct SeriesRegistration {
	CSegSetWithImages& input_set; 
	C2DImageSeries& input_images; 
	string minimizer; 
	const std::vector<const char *>& costs; 
	size_t mg_levels; 
	P2DTransformationFactory transform_creator; 
	int reference; 

	SeriesRegistration(CSegSetWithImages& _input_set, 
			   C2DImageSeries& _input_images, 
			   const string& _minimizer, 
			   const std::vector<const char *>& _costs, 
			   size_t _mg_levels, 
			   P2DTransformationFactory _transform_creator, 
			   int _reference
		):
		input_set(_input_set),
		input_images(_input_images), 
		minimizer(_minimizer), 
		costs(_costs),
		mg_levels(_mg_levels), 
		transform_creator(_transform_creator), 
		reference(_reference)
		{
		}
	void operator()( const blocked_range<int>& range ) const {
		CThreadMsgStream thread_stream;
		TRACE_FUNCTION; 
		auto m =  CMinimizerPluginHandler::instance().produce(minimizer);
		CSegSetWithImages::Frames& frames = input_set.get_frames(); 
		
		
		for( int i=range.begin(); i!=range.end(); ++i ) {
			if (i == reference)
				continue; 
			cvmsg() << "Register " << i << " to " << reference << "\n"; 
			auto cost  = create_costs(costs, i); 
			C2DNonrigidRegister nrr(cost, m,  transform_creator,  mg_levels, i);
			P2DTransformation transform = nrr.run(input_images[i], input_images[reference]);
			input_images[i] = (*transform)(*input_images[i]);
			frames[i].inv_transform(*transform);
		}
	}
};  

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg");
	string out_filename;
	
	auto transform_creator = C2DTransformCreatorHandler::instance().produce("spline"); 
	
	// registration parameters
	string minimizer("gsl:opt=gd,step=0.1");
	auto interpolator_kernel = produce_spline_kernel("bspline:d=3");
	size_t mg_levels = 3; 
	int reference_param = -1; 
	int skip = 0; 
	
	int max_threads = task_scheduler_init::automatic;
	
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-filebase", 0, "file name for registered fiels")); 

	options.set_group("\nRegistration"); 
	options.add(make_opt( skip, "skip", 'k', "Skip images at the beginning of the series"));
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( transform_creator, "transForm", 'f', "transformation type"));
	options.add(make_opt( reference_param, "ref", 'r', "reference frame (-1 == use image in the middle)")); 

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 't', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  
	

	if (options.parse(argc, argv, "cost") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	task_scheduler_init init(max_threads);
	
	CSegSetWithImages  input_set(in_filename, true);
	C2DImageSeries input_images = input_set.get_images(); 
	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 
	
	
	// if reference is not given, use half range 
	size_t reference = reference_param < 0  ? input_images.size() / 2 : reference_param; 
	
	if ( input_images.empty() ) 
		throw invalid_argument("No input images to register"); 
	
	if (reference > input_images.size() - 1) {
		reference = input_images.size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	if (skip > reference - 1) {
		throw invalid_argument("Skipping past reference\n");  
	}
	
	SeriesRegistration sreg(input_set, input_images, minimizer, cost_functions, 
				mg_levels, transform_creator, reference); 
	
	parallel_for(blocked_range<int>( skip, input_images.size()), sreg);
	
	
	input_set.set_images(input_images); 									  
	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main); 
