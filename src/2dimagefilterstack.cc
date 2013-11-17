/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>
#include <mia/core.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;

size_t log10(size_t x)
{
	int result = 0;
	while (x > 0) {
		++result;
		x /= 10;
	}
	return result;
}

const SProgramDescription g_general_help = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
        {pdi_short,	"Run filters on a series of 2D images."}, 
	{pdi_description, 	"This program runs a series filters on a series of "
	 "consecutive numbered input image. The filters are given as extra parameters "
	 "on the command line and are run in the order in which they are given."}, 
	{pdi_example_descr, "Run a kmeans classification of 5 classes on images inputXXXX.png "
	 "(X being digits) and then a binarization of the 4th class and store the result in "
	 "resultXXXX.png"}, 
	{pdi_example_code, "-i input0000.png -o result -t png kmeans:c=5 binarize:min=4,max=4"}
}; 

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;

	size_t startid = 0; 
	size_t endid = numeric_limits<int>::max(); 
		
	const C2DFilterPluginHandler::Instance& filter_plugins = C2DFilterPluginHandler::instance();
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
		
	
	CCmdOptionList options(g_general_help);

	options.set_group("File IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name base, the file type is set "
			      " accurding to the 'type' option", CCmdOption::required, &imageio));
	options.add(make_opt( out_type, imageio.get_supported_suffix_set(), "type", 't',"output file type, if "
			      "not given the input type is used"));

	options.set_group("Processing1"); 
	options.add(make_opt(startid, "start", 's', "first possible number of file number range to be filtered")); 
	options.add(make_opt(endid, "end", 'e', "last possible number of file number range to be filtered")); 
	
		
	options.set_group(g_help_optiongroup); 
	options.add(make_help_opt( "help-filters", 0,
				   "give some help about the filter plugins", 
				   new TPluginHandlerHelpCallback<C2DFilterPluginHandler>)); 
		
	if (options.parse(argc, argv, "filter", &filter_plugins) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
		
		
	auto filter_chain = options.get_remaining();
		
	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
	cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

	if ( filter_chain.empty() )
		cvwarn() << "no filters given, just copy\n";

	if ( in_filename.empty() )
		throw runtime_error("'--in-file' ('i') option required");

	if ( out_filename.empty() )
		throw runtime_error("'--out-base' ('o') option required");

	bool use_src_format = out_type.empty();

	if (!imageio.preferred_plugin_ptr(out_type)) {
		cvwarn() << "Output file format '"<< out_type << "' not supported, revert to input file format\n";
		use_src_format = true;
	}


	//	CHistory::instance().append(argv[0], revision, options);

	list<P2DFilter> filters;

	for (auto i = filter_chain.begin();
	     i != filter_chain.end(); ++i) {
		cvdebug() << "Prepare filter " << *i << endl;
		auto filter = filter_plugins.produce(*i);
		if (!filter){
			stringstream error;
			error << "Filter " << *i << " not found";
			throw invalid_argument(error.str());
		}
		filters.push_back(filter);
	}

	size_t start_filenum = startid;
	size_t end_filenum  = endid;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);

	if (end_filenum > endid) 
		end_filenum = endid; 
	
	char new_line = cverb.show_debug() ? '\n' : '\r';
#ifndef WIN32
	time_t start_time = time(NULL);
#endif
	for (size_t i = start_filenum; i < end_filenum; ++i) {

		string src_name = create_filename(src_basename.c_str(), i);
		cvmsg() << new_line << "Filter: " << i <<" out of "<< "[" << start_filenum<< "," << end_filenum << "]" ;
		C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);

		if (in_image_list.get() && in_image_list->size()) {

			if (use_src_format)
				out_type = in_image_list->get_source_format();

			auto filter_name = filter_chain.begin();

			for (auto f = filters.begin(); f != filters.end(); ++f, ++filter_name) {
				cvdebug() << "Run filter: " << *filter_name << "\n";
				for (auto i = in_image_list->begin();  i != in_image_list->end(); ++i) {
					*i = (*f)->filter(**i);
				}
			}

			stringstream ss;
			ss << out_filename << setw(format_width) << setfill('0') << i << "." << out_type;
			cvdebug() << "Save to " << ss.str() << ", format = " << out_type << "\n";

			if ( !imageio.save(ss.str(), *in_image_list) ){
				string not_save = ("unable to save result to ") + ss.str();
				throw runtime_error(not_save);

			}
		}
#ifndef WIN32
		if (cverb.shows(vstream::ml_message)) {
			char esttime[30];
			time_t est_end = (( end_filenum - start_filenum) * (time(NULL) - start_time)) /
				(i - start_filenum + 1) + start_time;
			ctime_r(&est_end, esttime);
			char *est = esttime;
			while (*est != '\n' && *est != 0)
				++est;
			if (*est == '\n')
				*est = ' ';
			cvmsg() << ", estimated finish at: " <<  esttime;
		}
#endif
	}
	cvmsg() << "\n";
	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
