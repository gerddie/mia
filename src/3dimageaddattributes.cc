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

  \subsection{mia-3dimageaddattributes}
  \label{mia-3dimageaddattributes}
  
  \begin{description}
  \item [Description:] This program appends all the meta-data attributes found in a 2D input image 
    to a 3D image (if the output format supports such attributes). 
    Attributes that are already present in the 3D image are not overwritten. 
  \
  \begin{lstlisting}
mia-3dimageaddattributes -i <input> -o <output> -a <2D image with attributes>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{attr}{a}{string}{2D image to get the attributes from}
  \opttypethreed
  }

  \item [Example:] Append the attributes found in 2d.v to 3d.v and store the result in attr3d.v 
   \
  \begin{lstlisting}
mia-3dimageaddattributes -i 3d.v -o attr3d.v -a 2d.v 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include <mia/3d/3dimageio.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const char *g_description = 
	"This program is used to add metadata attributes to a 3D image file (if supported)\n"
	"Basic usage:\n"
	"  mia-3dimageaddattributes [options] \n"; 


int main( int argc, const char *argv[] )
{

	try {
	string in_filename;
	string out_filename;
	string attr_image;

	const C2DImageIOPluginHandler::Instance& image2dio = C2DImageIOPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
				    "input image(s) to be filtered", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.add(make_opt( attr_image, "attr", 'a',
				    "2D image providing the attributes", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";

	//CHistory::instance().append(argv[0], "unknown", options);

	// read image
	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	C2DImageIOPluginHandler::Instance::PData  attr_image_list = image2dio.load(attr_image);


	if ( in_image_list.get() && in_image_list->size() &&
	     attr_image_list.get() && attr_image_list->size() ) {

		P2DImage attr_image = *attr_image_list->begin();
		PAttributeMap attr = attr_image->get_attribute_list();
		for (C3DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
			     i != in_image_list->end(); ++i)
			for (CAttributeMap::const_iterator a = attr->begin();
			     a != attr->end(); ++a) {
				if (!(*i)->has_attribute(a->first))
					(*i)->set_attribute(a->first, a->second);
			}

	}
	if ( !imageio.save(out_filename, *in_image_list) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	}

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

