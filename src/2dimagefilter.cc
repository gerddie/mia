/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

  \optiontable{
  \optinfile
  \optoutfile
  \opthelpplugin
  }

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

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescrption g_general_help = {
	"Image Filtering", 
	
	"This program runs a series filters on a given input image. The filters are given as extra parameters "
	"on the command line and are run in the order in which they are given.", 
	
	"Run a kmeans classification of 5 classes on input.png and then a binarization of the 4th class and store the result"
	" in result.png", 
	
	"-i input.png -o result.png kmeans:c=5 binarize:min=4,max=4"
}; 

int do_main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename;

	const auto& filter_plugins = C2DFilterPluginHandler::instance();
	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output image(s) that have been filtered", CCmdOption::required));
	options.set_group(g_help_optiongroup); 
	options.add(make_help_opt( "help-filters", 0,
				   "give some help about the filter plugins", 
				   new TPluginHandlerHelpCallback<C2DFilterPluginHandler>));
	
	if (options.parse(argc, argv, true) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto filter_chain = options.get_remaining();

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
	cvdebug() << "supported filters: " << filter_plugins.get_plugin_names() << "\n";

	if ( filter_chain.empty() )
		cvwarn() << "no filters given, just copy\n";


	//CHistory::instance().append(argv[0], "unknown", options);

	bool want_help = false; 
	std::vector<P2DFilter> filters(filter_chain.size());
	transform(filter_chain.begin(), filter_chain.end(), filters.begin(),
		  [&filter_plugins, &want_help](const char * name) {
			  auto filter =  filter_plugins.produce(name); 
			  if (!filter) {
				  if (name == plugin_help) 
					  want_help = true; 
				  else 
					  THROW(invalid_argument, "Filter '" << name << "' not found"); 
			  }
			  return filter; 
		  }
		); 
		
	auto in_image_list = imageio.load(in_filename);
	if (!in_image_list || in_image_list->empty()) {
		THROW(invalid_argument, "No images found in " << in_filename); 
	}
	
	for (auto f = filters.begin(); f != filters.end(); ++f) {
		cvmsg() << "Run filter: " << (*f)->get_init_string() << "\n";
		
		transform(in_image_list->begin(), in_image_list->end(), in_image_list->begin(),
			  [f](const P2DImage& img){return  (*f)->filter(*img);}); 
	}
	
	if ( !imageio.save(out_filename, *in_image_list) ){
		THROW(runtime_error, "Unable to save result to " << out_filename);
	};
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 
