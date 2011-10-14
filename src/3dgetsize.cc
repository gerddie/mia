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
  LatexBeginProgramDescription{Miscellaneous programs}

  \subsection{mia-3dgetsize}
  \label{mia-3dgetsize}

  
  \begin{description}
  \item [Description:] This program prints out the size of a 3D image as 3D vector <NX,NY,NZ>. 
  \
  \begin{lstlisting}
mia-3dgetsize -i <input> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  }

  \item [Example:] Print out the size of image.v: 
   \
  \begin{lstlisting}
mia-3dgetsize -i image.v 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/



#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#include <mia/3d/3dimageio.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescrption g_description = {
	"Miscellaneous programs", 
	
	"Write the dimensions of the input 3d image to stdout.", 
	
	"Print out the size of image.v.", 
	
	"-i image.v"
}; 

int main( int argc, const char *argv[] )
{
	string in_filename;
	try {
		const C3DImageIOPluginHandler::Instance& imageio3d = C3DImageIOPluginHandler::instance();

		CCmdOptionList options(g_description);
		options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
					    CCmdOption::required));

		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 

		C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio3d.load(in_filename);


		if (in_image_list.get() && in_image_list->size()) {
			cout << (*in_image_list->begin())->get_size() << "\n";
			return EXIT_SUCCESS;
		}
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

