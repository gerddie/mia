/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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
  LatexBeginProgramSection{2D image processing}
  \label{sec:2dimageproc}  
  
  

  LatexEnd
*/

/*
  LatexBeginProgramDescription{2D image processing}

  \subsection{mia-2dimagefilter}
  \label{mia-2dimagefilter}
  
  \begin{description}
  \item [Description:] This program is used to filter and convert 2D gray scale images by running 
  filters as given on the command line. For available filters see section \ref{sec:filter2d}. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagefilter -i <input image> -o <output image> [<filter>] ... 
  \end{lstlisting}
  with the filters given as extra parameters as additional command line parameters. 

  \item [Options:] $\:$

  \tabstart
  \optinfile
  \optoutfile
  \opthelpplugin
  \tabend

  \item [Example:]Run a mean-least-varaiance filter on input.exr, then run a 5-class k-means classification 
           and binarize by selecting the 4th class.
   \
  \begin{lstlisting}
mia-2dimagefilter -i image.exr -o filtered.png mlv:w=2 \
                                               kmeans:c=5 \
		                               binarize:min=4,max=4 
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;


static const char *program_info = 
	"This program is used to filter and convert gray scale images.\n"
	"Basic usage:\n"
	"  mia-2dimagefilter -i <input image> -o <output image> [<plugin>] ...\n"; 

int do_main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;
	; 

	const C2DFilterPluginHandler::Instance& filter_plugins = C2DFilterPluginHandler::instance();
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	stringstream filter_names;

	filter_names << "filters in the order to be applied (out of: " << filter_plugins.get_plugin_names() << ")";

	CCmdOptionList options(program_info);
	options.push_back(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',
				    "output file type (if not given deduct from output file name)"));
	options.set_group(g_help_optiongroup); 
	options.push_back(make_help_opt( "help-plugins", 0,
					 "give some help about the filter plugins", 
					 new TPluginHandlerHelpCallback<C2DFilterPluginHandler>));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	vector<const char *> filter_chain = options.get_remaining();

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
	cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

	if ( filter_chain.empty() )
		cvwarn() << "no filters given, just copy\n";


	//CHistory::instance().append(argv[0], "unknown", options);

	std::list<C2DFilterPlugin::ProductPtr> filters;

	for (std::vector<const char *>::const_iterator i = filter_chain.begin();
	     i != filter_chain.end(); ++i) {
		cvdebug() << "Prepare filter '" << *i << "'\n";
		C2DFilterPlugin::ProductPtr filter = filter_plugins.produce(*i);
		if (!filter){
			std::stringstream error;
			error << "Filter '" << *i << "' not found";
			throw invalid_argument(error.str());
		}
		filters.push_back(filter);
	}

	C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	if (in_image_list.get() && in_image_list->size()) {
		std::vector<const char *>::const_iterator filter_name = filter_chain.begin();
		for (std::list<C2DFilterPlugin::ProductPtr>::const_iterator f = filters.begin();
		     f != filters.end(); ++f, ++filter_name) {
			cvmsg() << "Run filter: " << *filter_name << "\n";
			for (C2DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
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

int main( int argc, const char *argv[] )
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
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
