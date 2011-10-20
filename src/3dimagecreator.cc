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

  \subsection{mia-3dimagecreator}
  \label{mia-3dimagecreator}
  
  \begin{description}
  \item [Description:] This program creates a 3D image that contains an object created by one of the object  
     creator plug-ins (section \ref{sec:creator3d}). 
  
  The program is called like 
  \
  \begin{lstlisting}
mia-3dimagecreator -i <input> -o <output> -n <number> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optoutfile
  \cmdopt{size}{s}{3D vector <uint>}{size of the output image}
  \optrepn
  \cmdopt{object}{j}{string}{object to be created (see section \ref{sec:creator3d})}
  \opttypethreed
  }

  \item [Example:] Create an image output.v of size $64\times128\times256$ that contains a lattic with 
                   frequencys 8, 16, and 4 in x, y and, z-direction respectively. 
   \
  \begin{lstlisting}
mia-3dimagecreator -o lattic.v -j lattic:fx=8,fy=16,fz=4 -s "<64,128,256>"
  \end{lstlisting}
  \end{description}

  LatexEnd
*/



#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <iomanip>

#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescrption g_description = {
	"Miscellaneous programs", 
	
	"This program creates a 3D image that contains an object created by one of the object "  
	"creator plug-ins (creator/3dimage)",
	
	"Create an image output.v of size <64,128,256> that contains a lattic with "
	"frequencys 8, 16, and 4 in x, y and, z-direction respectively.",
	
	"-o lattic.v -j lattic:fx=8,fy=16,fz=4 -s \"<64,128,256>\""
}; 
	
	
	

int do_main(int argc, char *argv[])
{
	string object("sphere");
	string out_filename;
	EPixelType pixel_type = it_ubyte;
	C3DBounds size(128,128,128);

	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();
	const C3DImageCreatorPluginHandler::Instance& creator_ph = C3DImageCreatorPluginHandler::instance();
	CCmdOptionList options(g_description);

	options.add(make_opt( out_filename, "out-file", 'o', "output file for create object", CCmdOption::required));
	options.add(make_opt( size, "size", 's', "size of the object"));
	options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type "));
	options.add(make_opt( object,  "object", 'j', "object to be created"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto creator = creator_ph.produce(object.c_str());
	if (!creator) {
		std::stringstream error;
		error << "Creator " << object << " not found";
		throw invalid_argument(error.str());
	}
	P3DImage image = (*creator)(size, pixel_type);
	if (!image) {
		std::stringstream error;
		error << "Creator " << object << " could not create object of size " << size 
		      << " and type " << CPixelTypeDict.get_name(pixel_type);
		throw invalid_argument(error.str());
	}

	C3DImageVector out_images;
	out_images.push_back(image);
	return !imageio.save(out_filename, out_images);
}



#include <mia/internal/main.hh>
MIA_MAIN(do_main)
