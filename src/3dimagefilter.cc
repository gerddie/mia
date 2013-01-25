/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Filter a 3D images."}, 
	{pdi_description, "This program is used to filter and convert gray scale 3D images."}, 
	{pdi_example_descr, "Run a mean-least-varaiance filter on input.v, then "
	 "run a 5-class k-means classification and binarize by selecting the 4th class."}, 
	{pdi_example_code, "-i image.v -o filtered.v mlv:w=2 kmeans:c=5 binarize:min=4,max=4" }
}; 


int do_main( int argc, char *argv[] )
{
	string in_filename;
	string out_filename;

	const auto& filter_plugins = C3DFilterPluginHandler::instance();
	const auto& imageio = C3DImageIOPluginHandler::instance();

	stringstream filter_names;

	filter_names << "filters in the order to be applied (out of: " << filter_plugins.get_plugin_names() << ")";

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output image(s) that have been filtered", 
			      CCmdOption::required, &imageio));
	options.add(make_help_opt( "help-plugins", 0, "give some help about the filter plugins", 
				   new TPluginHandlerHelpCallback<C3DFilterPluginHandler>)); 

	if (options.parse(argc, argv, "filter", &filter_plugins) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto filter_chain = options.get_remaining();

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
	cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

	if ( filter_chain.empty() )
		cvwarn() << "no filters given, just copy\n";

	if ( in_filename.empty() )
		throw runtime_error("'--in-image' ('i') option required");

	if ( out_filename.empty() )
		throw runtime_error("'--out-image' ('o') option required");

	//CHistory::instance().append(argv[0], "unknown", options);

	auto filters = create_filter_chain(filter_chain);

	// read image
	auto  in_image_list = imageio.load(in_filename);
	if (!in_image_list) 
		throw create_exception<runtime_error>("Unable to read 3D image from '", in_filename, "'");

	if (in_image_list->empty()) 
		throw create_exception<runtime_error>("Got empty image list from '", in_filename, "'");

	auto filter_name = filter_chain.begin();
	for (auto f = filters.begin();  f != filters.end(); ++f, ++filter_name) {
		cvmsg() << "Run filter: " << *filter_name << "\n";
		for (auto i = in_image_list->begin(); i != in_image_list->end(); ++i)
			*i = (*f)->filter(**i);
	}
	cvdebug() << "Save image to '" << out_filename << "\n"; 
	
	if ( !imageio.save(out_filename, *in_image_list) ){
		throw create_exception<runtime_error>("Unable to save result to '", out_filename, "'");
	}
	
	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
