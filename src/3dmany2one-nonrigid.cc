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
  LatexBeginProgramDescription{3D registration of series of images}
  
  \subsection{mia-3dmany2one-nonrigid}
  \label{mia-3dmany2one-nonrigid}

  \begin{description} 
  \item [Description:] 
  This program runs the non-rigid registration of an image series. 
  All images are registered to one refernces as given on the command line. 
  If no reference is given then the image in the middle of the series is selected. 
  Registration can be run in parallel.
  
  The program is called like 
  \begin{lstlisting}
mia-3dmany2one-nonrigid -i <input set> -o <output set> <cost1> [<cost2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input images}
  \cmdopt{out-file}{o}{string}{output images}
				 
  \cmdgroup{Image registration} 
  \cmdopt{ref}{r}{int}{Reference frame to base the registration on}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
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
mia-3dmany2one-nonrigid  -i segment.set -o registered.set -F spline:rate=16 \
                     image:cost=[ngf:eval=ds],weight=2.0 image:cost=ssd,weight=0.1 divcurl:weight=2.0 
  \end{lstlisting}
  \item [See also:] \sa{mia-3dserial-nonrigid}, \sa{mia-3dprealign-nonrigid}, 
                    \sa{mia-3dmotioncompica-nonrigid}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "3dmany3one"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/filetools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/3dimageio.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

using namespace tbb;
using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program runs the non-rigid registration of an perfusion image series. "
	"The registration is run in a serial manner, this is, only images in "
	"temporal succession are registered, and the obtained transformations "
	"are applied accumulated to reach full registration. "
  	"Basic usage: \n"
	" mia-3dmany2one-nonrigid [options] <cost1> <cost2> ..."; 


C3DFullCostList create_costs(const std::vector<const char *>& costs, int idx)
{
	stringstream cost_descr; 
	cost_descr << ",src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	C3DFullCostList result; 

	for (auto c = costs.begin(); c != costs.end(); ++c) {
		string cc(*c); 

		if (cc.find("image") == 0) 
			cc.append(cost_descr.str()); 
		cvdebug() << "create cost:"  << *c << " as " << cc << "\n"; 
		auto imagecost = C3DFullCostPluginHandler::instance().produce(cc);
		result.push(imagecost); 
	}

	return result; 
}

struct SeriesRegistration {
	C3DImageSeries&  input_images; 
	string minimizer; 
	const std::vector<const char *>& costs; 
	size_t mg_levels; 
	P3DTransformationFactory transform_creator; 
	int reference; 
	
	SeriesRegistration(C3DImageSeries&  _input_images, 
			   const string& _minimizer, 
			   const std::vector<const char *>& _costs, 
			   size_t _mg_levels, 
			   P3DTransformationFactory _transform_creator, 
			   int _reference):
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

		
		for( int i=range.begin(); i!=range.end(); ++i ) {
			if (i == reference)
				continue; 
			cvmsg() << "Register " << i << " to " << reference << "\n"; 
			auto cost  = create_costs(costs, i); 
			C3DNonrigidRegister nrr(cost, m,  transform_creator,  mg_levels, i);
			P3DTransformation transform = nrr.run(input_images[i], input_images[reference]);
			input_images[i] = (*transform)(*input_images[i]);
		}
	}
};  

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string registered_filebase("reg%04d.v");
	                        
	auto transform_creator = C3DTransformCreatorHandler::instance().produce("spline"); 

	// registration parameters
	string minimizer("gsl:opt=gd,step=0.1");
	auto interpolator_kernel = produce_spline_kernel("bspline:d=3");
	size_t mg_levels = 3; 
	int reference_param = -1; 

	int max_threads = task_scheduler_init::automatic;
	
	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "out-file", 'o', 
				    "file name for registered fiels")); 
	
	
	options.set_group("\nRegistration"); 
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
	
        // create cost function chain
	auto cost_functions = options.get_remaining(); 
	if (cost_functions.empty())
		throw invalid_argument("No cost function given - nothing to register"); 

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	P3DImageSeries input_images(new C3DImageSeries); 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		input_images->push_back(image);
	}

	// if reference is not given, use half range 
	size_t reference = reference_param < 0 ? input_images->size() / 2 : reference_param; 

	if ( input_images->empty() ) 
		throw invalid_argument("No input images to register"); 

	if (reference > input_images->size() - 1) {
		reference = input_images->size() - 1; 
		cvwarn() << "Reference was out of range, adjusted to " << reference << "\n"; 
	}

	SeriesRegistration sreg(*input_images, minimizer, cost_functions, 
				mg_levels, transform_creator, reference); 

	parallel_for(blocked_range<int>( 0, input_images->size()), sreg);

	bool success = true; 
	auto ii = input_images->begin(); 
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


