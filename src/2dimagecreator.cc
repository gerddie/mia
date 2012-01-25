/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

  \subsection{mia-2dimagecreator}
  \label{mia-2dimagecreator}
  
  \begin{description}
  \item [Description:] This program creates a 2D image that contains an object created by one of the object  
     creator plug-ins (section \ref{sec:creator2d}). 
  
  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagecreator -i <input> -o <output> -n <number> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optoutfile
  \cmdopt{size}{s}{2D vector <uint>}{size of the output image}
  \optrepn
  \cmdopt{object}{j}{string}{object to be created (see section \ref{sec:creator2d})}
  \opttypethreed
  }

  \item [Example:] Create an image output.v of size $64\times128\times256$ that contains a circle like 
                   object 
   \
  \begin{lstlisting}
mia-2dimagecreator -o circle.v -j circle:f=2,p=2 -s "<64,128,256>"
  \end{lstlisting}
  \end{description}

  LatexEnd
*/



#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
	"2D image processing", 
	
	"This program is used to create test images.", 
	
	"Create a 2D image containing a lattic with standard parameters of size <64,128> and pixel type"
	" 'float' and save the result to lattic.v", 
	
	"-o lattic.v -s '<64,128>' -r float -j lattic"
}; 

int do_main(int argc, char *argv[])
{
	C2DImageCreatorPluginHandler::ProductPtr creator;
	string out_filename;
	string type;
	EPixelType pixel_type = it_ubyte;
	C2DBounds size(128,128);

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	CCmdOptionList options(g_description);

	options.add(make_opt( out_filename, "out-file", 'o', "output file for create object", 
				    CCmdOption::required));
	options.add(make_opt( size, "size", 's', "size of the object"));
	options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type "));
	options.add(make_opt( creator, "", "object", 'j', "object to be created", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 



	P2DImage image = (*creator)(size, pixel_type);
	if (!image) {
		std::stringstream error;
		error << "Creator '" << creator->get_init_string() << "' could not create object of size " 
		      << size << " and type " << CPixelTypeDict.get_name(pixel_type);
		throw invalid_argument(error.str());
	}

	C2DImageVector out_images;
	out_images.push_back(image);
	return !imageio.save(out_filename, out_images);
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
