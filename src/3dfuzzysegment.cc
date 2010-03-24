/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: segment3d.cc 938 2006-07-11 11:57:01Z write1 $

/*! \brief mona-segment3d -- fuzzy clustering of 3D images

\par Program description

	Computes a fuzzy clustering to facilitate a 3D segmentation of 3D image.

\par Usage

	<code>mona-segment3d --usage</code> (or <code> mona-getmtr --help</code>)

	\param --in-file		image to be segmented
	\param --b0-image		image corected for intensity non-uniformities
	\param --cls-file		segmented classes to store probability density values
	\param --no-of-classes		no of classes to segment image
	\param --residuum		relative residuum to differentiate clusters
	\param --verbose		some verbose output (min/max values, etc)

\par Example

	-# Perform a segmentation (WM, GM, CSF) of a 3D T1w VISTA image
	\code
		user> mona-segment3d -i image.v -o b0-image.v -c cls.v -r 0.2 -n 3
	\endcode
	Note, the image format can be anything covered by the io plugins included in \a libmona.

\par Known bugs

	The program perfomance needs to be significantly improved..

\todo
	Program cannot cope with negative pixel values

\file segment3d.cc

\remark The programm was origionall written by D.L. Pham and J.L.Prince, Pat. Rec. Let., 20:57-68,1999.
\remark The CG optimization was included by Stefan Burckhardt und Carsten Walters

\author Heike J"anicke and M. Tittgemeyer (tittge@cbs.mpg.de), 2004
*/

// $Id: segment3d.cc 938 2006-07-11 11:57:01Z write1 $

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <mia/3d/fuzzyseg.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_USE
using namespace std;




int main( int argc, const char *argv[] )
{


	string in_filename;
	string out_filename;
	string cls_filename;
	string out_type;
	int    noOfClasses = 3;
	float  residuum = 0.1;


	try {

		CCmdOptionList options;
		options.push_back(make_opt( in_filename, "in-file", 'i',
					    "input image(s) to be segmenetd", "input", true));
		options.push_back(make_opt( cls_filename, "cls-file", 'c',
					    "output class probability images", "output", true));
		options.push_back(make_opt( out_filename, "b0-file", 'o',
					    "image corrected for intensity non-uniformity", NULL ));
		options.push_back(make_opt( noOfClasses, "no-of-classes", 'n',
					    "number of classes", "classes", false));
		options.push_back(make_opt( residuum, "residuum", 'r',
					    "relative residuum", "residuum", false));

		options.parse(argc, argv);

		vector<const char *> non_options = options.get_remaining();

		// what to do with unrecognized options
		if ( non_options.size() > 0 )
			throw invalid_argument("unknown options");

		// required options (anything that has no default value)
		if ( in_filename.empty() )
			throw runtime_error("'--in-file'  ('i') option required\n");
		if ( in_filename.empty() )
			throw runtime_error("'--cls-file' ('c') option required\n");

		const C3DImageIOPluginHandler::Instance&
			imageio = C3DImageIOPluginHandler::instance();

		C3DImageIOPluginHandler::Instance::PData inImage_list = imageio.load(in_filename);

		if (!inImage_list.get() || !inImage_list->size() ) {
			string not_found = ("No supported data found in ") + in_filename;
			throw runtime_error(not_found);
		}

		// segment image
		if (inImage_list->size() > 1)
			cvwarn() << "Only segmenting first input image\n";

		C3DImageVector classes;

		P3DImage b0_corrected = fuzzy_segment_3d(**inImage_list->begin(), noOfClasses, residuum, classes);

		if (!out_filename.empty()) {

			// save corrected image to out-file
			C3DImageIOPluginHandler::Instance::Data out_list;

			out_list.push_back(b0_corrected);
			if ( !imageio.save(out_type, out_filename, out_list) ){

				string not_save = ("unable to save result to ") + out_filename;
				throw runtime_error(not_save);

			};

		};

		//CHistory::instance().append(argv[0], revision, opts);

		if ( !imageio.save(out_type, cls_filename, classes) ){
			string not_save = ("unable to save result to ") + cls_filename;
			throw runtime_error(not_save);

		}


		return EXIT_SUCCESS;

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
