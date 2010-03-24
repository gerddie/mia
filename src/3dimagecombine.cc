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


#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

int do_main( int argc, const char *argv[] )
{

	string in_image1;
	string in_image2;
	string out_filename;
	string combiner_descr;
	bool help_plugins = false;

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;

	CCmdOptionList options;
	options.push_back(make_opt( in_image1, "image1", '1', "input image  1 to be combined", "image1", true));
	options.push_back(make_opt( in_image2, "image2", '2', "input image  2 to be combined", "image2", true));
	options.push_back(make_opt( combiner_descr, "combiner", 'c', "combiner operation", "combiner", true));
	options.push_back(make_opt( out_filename, "out", 'o', "output file", NULL, true));
	options.push_back(make_opt( help_plugins, "help-plugins", 'p', "give some help about the plugins", false));

	options.parse(argc, argv);

	if (help_plugins) {
		C3DImageCombinerPluginHandler::instance().print_help(cout);
		return EXIT_SUCCESS;
	}

	cvdebug() << "Load images from '" << in_image1 << "' and '" << in_image2 << "'\n";
	PImageVector image1list = imageio.load(in_image1);
	PImageVector image2list = imageio.load(in_image2);


	if (!image1list || image1list->size() < 1) {
		cverr() << "no image found in " << in_image1 << "\n";
		return EXIT_FAILURE;
	}

	if (!image2list || image2list->size() < 1) {
		cverr()  << "no image found in " << in_image2 << "\n";
		return EXIT_FAILURE;
	}

	if (image1list->size() > 1)
		cvwarn() << "only first image in " << in_image1 << "will be used\n";

	if (image2list->size() > 1)
		cvwarn() << "only first image in " << in_image2 << "will be used\n";

	P3DImageCombiner combiner =  C3DImageCombinerPluginHandler::instance().produce(combiner_descr.c_str());

	PCombinerResult combination = combiner->combine(**image1list->begin(), **image2list->begin());

	combination->save(out_filename);

	return 0;
};


int main(int argc, const char **args)
{
	try {
		return do_main(argc, args);
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n";
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n";
	}
	catch (exception& err) {
		cerr << "exception: " << err.what() << "\n";
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE;
}
