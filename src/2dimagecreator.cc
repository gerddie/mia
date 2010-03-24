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


int do_main(int argc, const char *args[])
{
	string object("sphere");
	string out_filename;
	string type;
	EPixelType pixel_type = it_ubyte;
	C2DBounds size(128,128);

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	const C2DImageCreatorPluginHandler::Instance& creator_ph = C2DImageCreatorPluginHandler::instance();
	CCmdOptionList options;

	options.push_back(make_opt( out_filename, "out-file", 'o', "output file for create object", "output", true));
	options.push_back(make_opt( type, imageio.get_set(), "type", 't', "Output file type", "filetype", false));
	options.push_back(make_opt( size, "size", 's', "size of the object", "size", false));
	options.push_back(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type ", "ubyte", false));
	options.push_back(make_opt( object,  "object", 'j', "object to be created", "object", false));

	options.parse(argc, args);

	if (!options.get_remaining().empty()) {
		cverr() << "Unknown arguments: ";
		for (vector<const char *>::const_iterator i = options.get_remaining().begin(); i !=  options.get_remaining().end();
		     ++i)
			cverb << *i << " ";
		cverb << "\n";
		return EXIT_FAILURE;
	}

	C2DImageCreatorPlugin::ProductPtr creator = creator_ph.produce(object.c_str());
	if (!creator) {
		std::stringstream error;
		error << "Creator " << object << " not found";
		throw invalid_argument(error.str());
	}
	P2DImage image = (*creator)(size, pixel_type);
	if (!image) {
		std::stringstream error;
		error << "Creator " << object << " could not create object of size " << size << " and type " << CPixelTypeDict.get_name(pixel_type);
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



