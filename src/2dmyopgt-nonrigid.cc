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
  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dmyopgt-nonrigid}
  \label{mia-2dmyopgt-nonrigid}

  \begin{description} \item [Description:] This program runs the
  non-linear registration of an perfusion image series as described in
  \cite{li09}.  According to the paper, a linear registration should
  be run before this program is applied.
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyopgt-nonrigid -i <input set> -o <output set> <cost1> [<cost2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
				 
  \cmdgroup{Pseudo Ground Thruth Estimation} 
  \cmdopt{alpha}{A}{float}{spacial neighborhood penalty weight}
  \cmdopt{beta}{B}{float}{temporal second derivative penalty weight}
  \cmdopt{rho-thresh}{C}{float}{crorrelation threshhold for neighborhood analysis}

  \cmdgroup{Image registration} 
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{interpolator}{p}{string}{Image interpolator to be used}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of registration passes to be run}
  \cmdopt{start-c-rate}{a}{float}{start coefficinet rate in spines, gets divided by \texttt{-{}-c-rate-divider} 
                                for each new registration pass}
  \cmdopt{c-rate-divider}{}{float}{cofficient rate divider for each pass}
  \cmdopt{start-divcurl}{d}{float}{start divcurl weight, gets divided by \texttt{-{}-divcurl-divider}
                                for each new registration pass}
  \cmdopt{ivcurl-divider}{}{float}{divcurl weight scaling with each new pass}
  \cmdopt{imageweight}{w}{float}{Weight for the image cost function}
  }

  \item [Example:]Register the perfusion series given in segment.set by using automatic ICA estimation. 
        Skip two images at the beginning and otherwiese use the default parameters. 
	Store the result in registered.set. 
  \begin{lstlisting}
mia-2dmyopgt-nonrigid  -i segment.set -o registered.set -k 2
  \end{lstlisting}

  \item [Remark:] This program is here for test purpouses only, the algorithm doesn't work very well. 
                  It should be better to implement the follow-up paper \citet{Li2011}. 
  \item [See also:] \sa{mia-2dmyomilles}, \sa{mia-2dmyoperiodic-nonrigid}, 
                    \sa{mia-2dmyoica-nonrigid}, \sa{mia-2dmyoserial-nonrigid},
		    \sa{mia-2dsegseriesstats}
  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "2dmyopgt"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <libxml++/libxml++.h>
#include <mia/core.hh>

#include <mia/core/errormacro.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ground_truth_evaluator.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>


using namespace std;
using namespace mia;

const char *g_description = 
	"This program is used Pseudo Ground Thruth for motion compensation "
	"of series of myocardial perfusion images as decribed in Chao Li and Ying Sun, "
	"'Nonrigid Registration of Myocardial Perfusion MRI Using Pseudo Ground Truth' , In Proc. "
	"Medical Image Computing and Computer-Assisted Intervention – MICCAI 2009, 165-172, 2009 "; 


C2DFullCostList create_costs(double divcurlweight, double imageweight)
{
	C2DFullCostList result; 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	stringstream image_descr; 
	image_descr << "image:weight=" << imageweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(image_descr.str())); 
	return result; 
}

P2DTransformationFactory create_transform_creator(size_t c_rate)
{
	stringstream transf; 
	transf << "spline:rate=" << c_rate; 
	return C2DTransformCreatorHandler::instance().produce(transf.str()); 
}

void run_registration_pass(CSegSetWithImages&  input_set, const C2DImageSeries& references, 
			   int skip_images, PMinimizer minimizer, 
			   C2DInterpolatorFactory& ipfactory, size_t mg_levels, 
			   double c_rate, double divcurlweight, double imageweight) 
{
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	C2DImageSeries input_images = input_set.get_images(); 
	auto costs  = create_costs(divcurlweight, imageweight); 
	auto transform_creator = create_transform_creator(c_rate); 
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, ipfactory, mg_levels);

	// this loop could be parallized 
	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register frame " << i << "\n"; 
		P2DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
		input_images[i + skip_images] = (*transform)(*input_images[i + skip_images], ipfactory);
		frames[i + skip_images].inv_transform(*transform);
	}
	input_set.set_images(input_images); 
}


int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;
	size_t skip_images = 0; 

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	double c_rate = 32; 
	double c_rate_divider = 4; 
	double divcurlweight = 20.0; 
	double divcurlweight_divider = 4.0; 
	double imageweight = 1.0; 
	auto interpolator_kernel = produce_spline_kernel("bspline:d=3");
	size_t mg_levels = 3; 
	size_t current_pass = 0; 
	size_t max_pass = 4; 

	// Pseudo Ground Thruth estimation parameters 
	double alpha = 1.0;
	double beta = 1.0;
	double rho_thresh = 0.85;

	CCmdOptionList options(g_description);

	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', "file name base for registered fiels")); 

	options.set_group("\nRegistration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( c_rate, "start-c-rate", 'a', 
				    "start coefficinet rate in spines, gets divided by --c-rate-divider with every pass"));
	options.add(make_opt( c_rate_divider, "c-rate-divider", 0, 
				    "cofficient rate divider for each pass"));
	options.add(make_opt( divcurlweight, "start-divcurl", 'd',
				    "start divcurl weight, gets divided by --divcurl-divider with every pass")); 
	options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
				    "divcurl weight scaling with each new pass")); 
	options.add(make_opt( imageweight, "imageweight", 'w', "image cost weight")); 
	options.add(make_opt( interpolator_kernel ,"interpolator", 'p', "image interpolator kernel"));
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( max_pass, "passes", 'P', "registration passes")); 

	options.set_group("\nPseudo Ground Thruth estimation"); 
	options.add(make_opt( alpha, "alpha", 'A', "spacial neighborhood penalty weight"));
	options.add(make_opt( beta, "beta", 'B', "temporal second derivative penalty weight"));
	options.add(make_opt( rho_thresh, "rho-thresh", 'R', 
				    "crorrelation threshhold for neighborhood analysis"));

	
	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	P2DInterpolatorFactory ipfactory(new C2DInterpolatorFactory(ipf_spline, interpolator_kernel));

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 

	// sanity check 
	if (input_images.size() < 5 + skip_images) {
		THROW(invalid_argument, "input set has only " << input_images.size() << " frames, but at least " 
		      << 5 + skip_images << " frames are required for the registration to make sense"); 
	}
	
	// copy true perfusion set to temporary series
	vector<P2DImage> series(input_images.size() - skip_images);
	copy(input_images.begin() + skip_images, input_images.end(), series.begin()); 


	// prepare PGT evaluator 
	C2DGroundTruthEvaluator gte(alpha, beta, rho_thresh);
	vector<P2DImage> pgt;
	
	// run registration passes 
	do {
		cvmsg() << "Registration pass " << current_pass++ << "\n"; 

		gte(series, pgt);
		run_registration_pass(input_set, pgt,  skip_images,  minimizer, 
				      *ipfactory, mg_levels, c_rate, divcurlweight, imageweight); 
		
		divcurlweight /= divcurlweight_divider; 
		if (c_rate > 1) 
			c_rate /= c_rate_divider; 
	} while (current_pass < max_pass); 

	// copy back registration results 
	copy(series.begin(), series.end(), input_images.begin() + skip_images); 
	
	// save results
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
