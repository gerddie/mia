/*
 * Copyright (c) 2004 Max-Planck-Institute of Evolutionary Anthropology
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

// $Id: get_reminder.cc,v 1.5 2006-07-12 13:44:29 wollny Exp $

/*! \brief eva-2dimagefilter

\sa 3va-2dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/2dimageio.hh>


using namespace std;
NS_MIA_USE

P2DImage combine_and_invert(const C2DBitImage& a, const C2DBitImage& b)
{
	assert(a.get_size() == b.get_size()); 
	C2DBitImage *result = new C2DBitImage(a.get_size()); 
	
	transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_or<bool>())); 
	return P2DImage(result); 
}

/* Revision string */
const char g_description[] = "This program is used to combine two binary images and invert the result";

int main( int argc, const char *argv[] )
{

	string filename1;
	string filename2;
	string out_filename;

	CCmdOptionList options(g_description);
	
	options.push_back(make_opt( filename1, "file1", '1', 
				    "input mask image 1", NULL, CCmdOption::required)); 
	options.push_back(make_opt( filename2, "file2", '2', 
					"input input mask image 2", NULL, CCmdOption::required)); 
	options.push_back(make_opt( out_filename, "out-file", 'o', 
					"output mask image", NULL, CCmdOption::required)); 

	try {

		options.parse(argc, argv, false);
		
		// read images
		P2DImage image1 = load_image2d(filename1); 
		P2DImage image2 = load_image2d(filename2); 

		try {
			const C2DBitImage& img1 = dynamic_cast<const C2DBitImage&>(*image1); 
			const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*image2); 
			P2DImage result = combine_and_invert(img1, img2); 
			
			if ( !save_image(out_filename, result) ){
				THROW(runtime_error, "cannot save result to " << out_filename); 
			}
			
		}catch (bad_cast& x) {
			throw invalid_argument("Input images are not binary masks"); 
		}
		
		return EXIT_SUCCESS; 
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
