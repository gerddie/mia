/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
  LatexBeginProgramDescription{Miscellaneous programs}

  \subsection{mia-3dimageselect}
  \label{mia-3dimageselect}
  
  \begin{description}
  \item [Description:] This program selects one image from a multi-image file and writes it to a seperate file. 
  The program is called like 
  \
  \begin{lstlisting}
mia-3dimageselect -i <input> -o <output> -n <number> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{number}{n}{int}{Number of image to be selected. Counting begins at zero}
  \opttypethreed
  }

  \item [Example:] Store the third image in multiimage.v to image.v: 
   \
  \begin{lstlisting}
mia-3dimageselect -i multiimage.v -o image.v -n 2 
  \end{lstlisting}
  \end{description}

  LatexEnd
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


static const char *program_info = 
	"This program is used to select one 3D images from a multi-image file.\n"
	"Basic usage:\n"
	"  mia-3dimageselect -i <input image> -o <output image> [<options>] ...\n"; 

int do_main( int argc, const char *argv[])
{

	string in_filename;
	string out_filename;
	size_t num = 0;

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


	CCmdOptionList options(program_info);
	options.add(make_opt( in_filename, "in-file", 'i',
				    "input images", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image", CCmdOption::required));
	options.add(make_opt( num, "number", 'n',  "image number to be selected"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	// read image
	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	if (!in_image_list) {
		stringstream msg;
		msg << "No images found in '" << in_filename <<"'";
		throw invalid_argument(msg.str());
	}

	if (!( num < in_image_list->size() )) {
		stringstream msg;
		msg << "Request image nr. " << num << ", but input file '"<< in_filename
		    <<"'has only " << in_image_list->size() << " images";
		throw invalid_argument(msg.str());
	}

	C3DImageIOPluginHandler::Instance::Data out_image_list;
	out_image_list.push_back( (*in_image_list)[num]);

	if ( !imageio.save(out_filename, out_image_list) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};

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

