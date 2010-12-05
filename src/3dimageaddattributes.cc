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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// $Id: 3dimagefilter.cc,v 1.12 2006-07-12 13:44:23 wollny Exp $

/*! \brief mia-3dimagefilter

\sa mia-3dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/3dimageio.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const char *g_description = 
	"This program is used to add metadata attributes to a 3D image file (if supported)\n"
	"Basic usage:\n"
	"  mia-3dimageaddattributes [options] \n"; 


int main( int argc, const char *argv[] )
{

	try {
	string in_filename;
	string out_filename;
	string attr_image;
	string out_type;

	const C2DImageIOPluginHandler::Instance& image2dio = C2DImageIOPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.push_back(make_opt( in_filename, "in-file", 'i',
				    "input image(s) to be filtered", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',
				    "output file type"));
	options.push_back(make_opt( attr_image, "attr", 'a',
				    "2D image providing the attributes", CCmdOption::required));

	options.parse(argc, argv);

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";

	//CHistory::instance().append(argv[0], "unknown", options);

	// read image
	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	C2DImageIOPluginHandler::Instance::PData  attr_image_list = image2dio.load(attr_image);


	if ( in_image_list.get() && in_image_list->size() &&
	     attr_image_list.get() && attr_image_list->size() ) {

		P2DImage attr_image = *attr_image_list->begin();
		PAttributeMap attr = attr_image->get_attribute_list();
		for (C3DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
			     i != in_image_list->end(); ++i)
			for (CAttributeMap::const_iterator a = attr->begin();
			     a != attr->end(); ++a) {
				if (!(*i)->has_attribute(a->first))
					(*i)->set_attribute(a->first, a->second);
			}

	}
	if ( !imageio.save(out_type, out_filename, *in_image_list) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
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

