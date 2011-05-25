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

NS_MIA_USE
using namespace boost;
using namespace std;

static const char *program_info = 
	"This program is used to create an image with some object.\n"
	"Basic usage:\n"
	"  mia-2dimagecreator <options>\n"; 


int do_main(int argc, const char *argv[])
{
	C2DImageCreatorPluginHandler::ProductPtr creator;
	string out_filename;
	string type;
	EPixelType pixel_type = it_ubyte;
	C2DBounds size(128,128);

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	CCmdOptionList options(program_info);

	options.add(make_opt( out_filename, "out-file", 'o', "output file for create object", 
				    CCmdOption::required));
	options.add(make_opt( size, "size", 's', "size of the object"));
	options.add(make_opt( pixel_type, CPixelTypeDict, "repn", 'r',"input pixel type "));
	options.add(make_opt( creator,  "object", 'j', "object to be created", CCmdOption::required));

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
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

int main(int argc, const char *argv[])
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
	catch (const std::exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}



