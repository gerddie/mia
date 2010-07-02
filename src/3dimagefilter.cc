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

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

int main( int argc, const char *argv[] )
{
	bool help_plugins = false;

	try {
	string in_filename;
	string out_filename;
	string out_type;

	const C3DFilterPluginHandler::Instance& filter_plugins = C3DFilterPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

	stringstream filter_names;

	filter_names << "filters in the order to be applied (out of: " << filter_plugins.get_plugin_names() << ")";

	CCmdOptionList options;
	options.push_back(make_opt( in_filename, "in-file", 'i',
				    "input image(s) to be filtered", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", "output", true));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',
				    "output file type" , "type", false));
	options.push_back(make_opt( help_plugins, "help-plugins", 0,
				    "give some help about the filter plugins", NULL));

	options.parse(argc, argv);

	vector<const char *> filter_chain = options.get_remaining();

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
	cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

	if (help_plugins) {
		filter_plugins.print_help(cout);
		return EXIT_SUCCESS;
	}

	if ( filter_chain.empty() )
		cvwarn() << "no filters given, just copy\n";

	if ( in_filename.empty() )
		throw runtime_error("'--in-image' ('i') option required");

	if ( out_filename.empty() )
		throw runtime_error("'--out-image' ('o') option required");

	//CHistory::instance().append(argv[0], "unknown", options);

	std::vector<C3DFilterPlugin::ProductPtr> filters = create_filter_chain(filter_chain);

	// read image
	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);


	if (in_image_list.get() && in_image_list->size()) {
		vector<const char *>::const_iterator filter_name = filter_chain.begin();
		for (vector<C3DFilterPlugin::ProductPtr>::const_iterator f = filters.begin();
		     f != filters.end(); ++f, ++filter_name) {
			cvmsg() << "Run filter: " << *filter_name << "\n";
			for (C3DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
			     i != in_image_list->end(); ++i)
				*i = (*f)->filter(**i);
		}

		if ( !imageio.save(out_type, out_filename, *in_image_list) ){
			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);
		};

	}
	return EXIT_SUCCESS;

	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		if (help_plugins) {
			C3DFilterPluginHandler::instance().print_help(cout);
			return EXIT_SUCCESS;
		}
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

