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

/*
  LatexBeginProgramDescription{3D image processing}

  \subsection{mia-3dimagefilterstack}
  \label{mia-3dimagefilterstack}
  
  \begin{description}
  \item [Description:] This program is used run the very same set of filters on 
    the individual images of a series of 3D gray scale images.
  For available filters see section \ref{sec:filter3d}. 

  The program is called like 
  \
  \begin{lstlisting}
mia-3dimagefilterstack -i <input images> -o <output image basename> \
                  -t <output file type> [<filter>] ... 
  \end{lstlisting}
  with the filters given as extra parameters as additional command line parameters. 

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \opttypethreed
  \opthelpplugin
  }

  \item [Example:]Run a mean-least-varaiance filter on a series of images that follow the 
  numbering pattern imageXXXX.exr and store the output in images filteredXXXX.exr 
   \
  \begin{lstlisting}
mia-3dimagefilterstack -i image0000.exr -o filtered -t exr mlv:w=2 
  \end{lstlisting}
  \item [Remark:] Note the difference to the program \hyperref[mia-3dstackfilter]{mia-3dstackfilter} 
                  that runs the filter(s) by combining the images in a 3D manner. 
  \end{description}
  
  LatexEnd
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/core.hh>

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

/* Revision string */
const char revision[] = "not specified";

static const char *program_info = 
	"This program is used to filter and convert a consecutive numbered series\n"
	"gray of scale images. File names must follow the pattern 'dataXXXX.v' \n"
	"(X being digits), i.e. the numbering comes right before the dot.\n"
	"\n"
	"Basic usage:\n"
	"  mia-3dimagefilter -i <input image> -o <output base> \\\n"
	"                    -t <output type> [<plugin>]  [<plugin>] ...\n"; 


int main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;
	vector<string> filter_chain;
	bool help_plugins = false;

	const C3DFilterPluginHandler::Instance& filter_plugins = C3DFilterPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();

	stringstream filter_names(program_info);

	filter_names << "filters in the order to be applied (out of: " << filter_plugins.get_plugin_names() << ")";


	CCmdOptionList options(program_info);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name base", CCmdOption::required));
	options.add(make_opt( out_type, imageio.get_set(), "type", 't',"output file type", CCmdOption::required));
	
	options.add(make_help_opt( "help-plugins", 0,
					 "give some help about the filter plugins", 
					 new TPluginHandlerHelpCallback<C3DFilterPluginHandler>)); 
	try {
		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 


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
			throw runtime_error("'--in-file' ('i') option required");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");

		bool use_src_format = out_type.empty();
		
		string out_suffix = imageio.get_preferred_suffix(out_type); 


		//	CHistory::instance().append(argv[0], revision, options);

		list<C3DFilterPlugin::ProductPtr> filters;

		for (vector<const char *>::const_iterator i = filter_chain.begin();
		     i != filter_chain.end(); ++i) {
			cvdebug() << "Prepare filter " << *i << endl;
			C3DFilterPlugin::ProductPtr filter = filter_plugins.produce(*i);
			if (!filter){
				stringstream error;
				error << "Filter " << *i << " not found";
				throw invalid_argument(error.str());
			}
			filters.push_back(filter);
		}

		size_t start_filenum = 0;
		size_t end_filenum  = 0;
		size_t format_width = 0;

		string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		if (start_filenum >= end_filenum)
			throw invalid_argument(string("no files match pattern ") + src_basename);

		char new_line = cverb.show_debug() ? '\n' : '\r';
#ifndef WIN32
		time_t start_time = time(NULL);
#endif
		for (size_t i = start_filenum; i < end_filenum; ++i) {

			string src_name = create_filename(src_basename.c_str(), i);
			cvmsg() << new_line << "Filter: " << i <<" out of "<< "[" << start_filenum<< "," << end_filenum << "]" ;
			C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);

			if (in_image_list.get() && in_image_list->size()) {

				if (use_src_format)
					out_type = in_image_list->get_source_format();

				vector<const char *>::const_iterator filter_name = filter_chain.begin();

				for (list<C3DFilterPlugin::ProductPtr>::const_iterator f = filters.begin();
				     f != filters.end(); ++f, ++filter_name) {
					cvdebug() << "Run filter: " << *filter_name << "\n";
					for (C3DImageVector::iterator i = in_image_list->begin();
					     i != in_image_list->end(); ++i) {
						*i = (*f)->filter(**i);
					}
				}

				stringstream ss;
				ss << out_filename << setw(format_width) << setfill('0') << i << "." << out_suffix;
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
