/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <boost/filesystem/convenience.hpp>


#include <mia/3d/fuzzyseg.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/datapool.hh>

NS_MIA_USE
using namespace std;
namespace bfs = ::boost::filesystem;

static const char *program_info = 
	"This program is used to mask the brain in T1 MR images by using fuzzy c-means\n"
	"combined with a series of other filters.\n"
	"Basic usage:\n"
	"  mia-3dimagefilter -i <input image> -o <output image> [<options>]\n"; 


int main( int argc, const char *argv[] )
{

	const char *b0poolkey = "b0.@";
	string in_filename;
	string out_filename;
	string out_type;
	int    noOfClasses = 3;
	int    wmclass = noOfClasses - 1;
	float  residuum = 0.1;
	int    growthresh = 20;
	string growshape("18n");
	float  wmclassprob = 0.7;

	try {

		CCmdOptionList options(program_info);
		options.push_back(make_opt( in_filename, "in-file", 'i',
					    "input image(s) to be segmented", CCmdOption::required));
		options.push_back(make_opt( out_filename, "out-file", 'o', "brain mask", CCmdOption::required ));
		options.push_back(make_opt( noOfClasses, "no-of-classes", 'n', "number of classes"));
		options.push_back(make_opt( wmclass,     "wm-class",      'w', "index of white matter"));
		options.push_back(make_opt( wmclassprob, "wm-prob", 'p',
					    "white matter class probability for initial mask creation"));
		options.push_back(make_opt( growthresh, "grow-threshold", 't', "intensity threshold for region growing"));
		options.push_back(make_opt( growshape, "grow-shape", 0, "neighbourhood mask region growing"));


		options.parse(argc, argv);

		vector<const char *> non_options = options.get_remaining();

		// what to do with unrecognized options
		if ( non_options.size() > 0 )
			throw invalid_argument("unknown options");

		// required options (anything that has no default value)
		if ( in_filename.empty() )
			throw runtime_error("'--in-file'  ('i') option required\n");
		if ( out_filename.empty() )
			throw runtime_error("'--out-file' ('c') option required\n");

		const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

		C3DImageIOPluginHandler::Instance::PData inImage_list = imageio.load(in_filename);

		if (!inImage_list.get() || !inImage_list->size() ) {
			string not_found = ("No supported data found in ") + in_filename;
			throw runtime_error(not_found);
		}

		if (wmclass >= noOfClasses)
			throw invalid_argument("number of classes should be smaller then number of classes");

		// segment image
		if (inImage_list->size() > 1)
			cvwarn() << "Only segmenting first input image\n";

		C3DImageVector classes;

		P3DImage b0_corrected = fuzzy_segment_3d(**inImage_list->begin(), noOfClasses, residuum, classes);
		CDatapool::Instance().add(b0poolkey, create_image3d_vector(b0_corrected));


		P3DImage result = classes[wmclass];

		// create filter chain
		vector<string> filter_chain;
		stringstream binarize;
		binarize << "binarize:min=" << wmclassprob;
		filter_chain.push_back(binarize.str());

		filter_chain.push_back("erode:shape=6n");
		filter_chain.push_back("label");
		filter_chain.push_back("selectbig");
		stringstream grow;
		grow << "growmask:ref="<< b0poolkey <<",min=" << growthresh << ",shape=" << growshape;
		filter_chain.push_back(grow.str());
		filter_chain.push_back("close:shape=[sphere:r=3]");
		filter_chain.push_back("open:shape=[sphere:r=3]");
		filter_chain.push_back(string("mask:input=") + b0poolkey);

		vector<C3DFilterPlugin::ProductPtr> filters = create_filter_chain(filter_chain);

		for (vector<C3DFilterPlugin::ProductPtr>::const_iterator f = filters.begin();
		     f != filters.end(); ++f) {
			result = (*f)->filter(*result);
		}


		if (!out_filename.empty()) {

			// save corrected image to out-file
			C3DImageIOPluginHandler::Instance::Data out_list;

			out_list.push_back(result);
			if ( !imageio.save(out_type, out_filename, out_list) ){

				string not_save = ("unable to save result to ") + out_filename;
				throw runtime_error(not_save);

			};

		};

		return EXIT_SUCCESS;

	}

	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}

	catch (const invalid_argument &e){
		cerr << argv[0] << " invalid argument: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " general error: " << e.what() << endl;
	}
 	catch (...){
 		cerr << argv[0] << " unknown exception" << endl;
 	}

	return EXIT_FAILURE;
}
