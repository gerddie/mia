/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

// $Id: 2dimagefilter.cc,v 1.12 2006-07-12 13:44:23 wollny Exp $

/*! \brief mia-2dimagecombiner

\file mia-2dimagefilter.cc
\author G. Wollny, gw.fossdev  at gmail.com, 2011

*/

#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE;
using namespace std;
using boost::any_cast; 


static const char *program_info = 
	"This program is used to combine two images.\n"
	"Basic usage:\n"
	"  mia-2dimagecombiner -1 <image 1> -2 <image 2>  -o <output image> [<plugin>]\n";

int do_main( int argc, const char *argv[] )
{

	string in1_filename;
	string in2_filename;
	string out_filename;

	auto combiner = C2DImageCombinerPluginHandler::instance().produce("absdiff");

	stringstream combiner_names;

	CCmdOptionList options(program_info);
	options.push_back(make_opt( in1_filename, "in-file-1", '1', 
				    "first input image to be combined", CCmdOption::required));
	options.push_back(make_opt( in2_filename, "in-file-2", '2', 
				    "second input image to be combined", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.push_back(make_opt( combiner, "operation", 'p', "operation to be applied"));
	
	options.parse(argc, argv, false);

	//CHistory::instance().append(argv[0], "unknown", options);

	auto image1 = load_image2d(in1_filename); 
	auto image2 = load_image2d(in2_filename); 

	auto output = combiner->combine(*image1, *image2); 
	if (!save_image(out_filename, outimage)) 
		THROW(runtime_error, "unable to save result in '" << out_filename << "'");
	
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

