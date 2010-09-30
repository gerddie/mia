/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <boost/lambda/lambda.hpp>
#include <mia/core.hh>

#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ground_truth_evaluator.hh>

using namespace std;
using namespace mia;

const char *g_description = 
	"This program is used to evaluate an estimate of Ground Thruth for motion compensation "
	"of series of myocardial perfusion images as decribed in Chao Li and Ying Sun, "
	"'Nonrigid Registration of Myocardial Perfusion MRI Using Pseudo Ground Truth' , In Proc. "
	" Medical Image Computing and Computer-Assisted Intervention – MICCAI 2009, 165-172, 2009 "; 

int do_main( int argc, const char *argv[] )
{
	string src_name("data0000.exr");
	string out_name("ref");
	string out_type("exr");
	size_t first = 2; 
	double alpha = 1.0;
	double beta = 1.0;
	double rho_thresh = 0.5;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	CCmdOptionList options(g_description);
	options.push_back(make_opt( src_name, "in-base", 'i', "input file name base", "input", false));
	options.push_back(make_opt( out_name, "out-base", 'o', "output file name base", "output", false));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',
				    "output file type" , "image-type"));

	options.push_back(make_opt( first, "skip", 's', "skip images at beginning of series", "skip", false));

	options.push_back(make_opt( alpha, "alpha", 'a', "spacial neighborhood penalty weight", "alpha", false));
	options.push_back(make_opt( beta, "beta", 'b', "temporal second derivative penalty weight", "beta", false));
	options.push_back(make_opt( rho_thresh, "rho_thresh", 'r', 
				    "crorrelation threshhold for neighborhood analysis", "rho", false));

	options.parse(argc, argv);

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(src_name, start_filenum, end_filenum, format_width);

	if (end_filenum - start_filenum < 5)
		throw invalid_argument ("This analysis doesn't make sense for series shorter then 5 images");

	vector<P2DImage> keep_series;
	vector<P2DImage> series;
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P2DImage image = load_image2d(src_name);
		if (!image)
			THROW(runtime_error, "image " << src_name << " not found");
		if (i >= first) 
			series.push_back(image);
		else 
			keep_series.push_back(image);
	}

	if (series.size() < 5) 
		THROW(runtime_error, "not enough input images found.");

	C2DGroundTruthEvaluator gte(alpha, beta, rho_thresh);
	vector<P2DImage> pgt;
	gte(series, pgt);

	for (size_t i = start_filenum; i < first; ++i) {
		stringstream fname;
		fname << out_name << setw(format_width) << setfill('0') << i << "." << out_type;
		if (!save_image2d(fname.str(), keep_series[i - start_filenum]))
			THROW(runtime_error, "unable to save '"<< fname.str() << "'");
	}
	
	for (size_t i = start_filenum + first; i < end_filenum; ++i) {
		stringstream fname;
		fname << out_name << setw(format_width) << setfill('0') << i << "." << out_type;
		if (!save_image2d(fname.str(), pgt[i - start_filenum-first]))
			THROW(runtime_error, "unable to save '"<< fname.str() << "'");
	}
	
	return EXIT_SUCCESS;
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

