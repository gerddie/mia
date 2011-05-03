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

#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

static const char *program_info = 
	"This program is used to create an image with some object.\n"
	"Basic usage:\n"
	"  mia-2dimagecreator <options>\n"; 


int do_main(int argc, const char *args[])
{
	C2DImageCreatorPluginHandler::ProductPtr creator;
	string out_filename;
	string type;
	EPixelType pixel_type = it_ubyte;
	C2DBounds size(128,128);

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	CCmdOptionList options(program_info);

	options.push_back(make_opt( out_filename, "out-file", 'o', "output file for create object", 
				    CCmdOption::required));
	options.push_back(make_opt( type, imageio.get_set(), "type", 't', 
				    "Output file type (normally deducted from output file name)"));
	options.push_back(make_opt( size, "size", 's', "size of the object"));
	options.push_back(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type "));
	options.push_back(make_opt( creator,  "object", 'j', "object to be created", CCmdOption::required));

	options.parse(argc, args, false);

	P2DImage image = (*creator)(size, pixel_type);
	if (!image) {
		std::stringstream error;
		error << "Creator '" << creator->get_init_string() << "' could not create object of size " 
		      << size << " and type " << CPixelTypeDict.get_name(pixel_type);
		throw invalid_argument(error.str());
	}

	C2DImageVector out_images;
	out_images.push_back(image);
	return !imageio.save(type, out_filename, out_images);
}

int main(int argc, const char *argv[])
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
	catch (const std::exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}



