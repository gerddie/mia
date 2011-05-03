/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#include <mia/3d/3dimageio.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>


using namespace std;
NS_MIA_USE;

static const char *program_info = 
	"Write the dimensions of the input 3d image to stdout.\n"
	"Usage:\n"
	"  mia-2dimagefilter -i <input image> \n"; 


int main( int argc, const char *argv[] )
{
	string in_filename;
	try {
		const C3DImageIOPluginHandler::Instance& imageio3d = C3DImageIOPluginHandler::instance();

		CCmdOptionList options(program_info);
		options.push_back(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
					    CCmdOption::required));

		options.parse(argc, argv);

		if ( !options.get_remaining().empty())
			throw invalid_argument("Unknown options given");

		if ( in_filename.empty() )
			throw runtime_error("'--in-image' ('i') option required");


		C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio3d.load(in_filename);


		if (in_image_list.get() && in_image_list->size()) {
			cout << (*in_image_list->begin())->get_size() << "\n";
			return EXIT_SUCCESS;
		}
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

