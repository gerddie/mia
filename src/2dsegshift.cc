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

  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dsegshift}
  \label{mia-2dsegshift}

  \begin{description} 
  \item [Description:] 
	This program move the segmentation(s) of an image series by using a shift 
          that is equal for all slices. 
	The program also may remove images from the begin of the series. 
        The program can be used to correct the segmentation of the images if 
          the images where cropped.  

  The program is called like 
  \begin{lstlisting}
mia-2dsegshift -i <input set> -o <output set> -g <output image name> \
               -S <shift> 
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{image-file}{g}{}{base name of the output image file. The numbering scheme and the file type of the 
      input images will be preserved.These files will not be touched and 
      should actually be the result of translation based linear registration.}
  \cmdopt{shift}{S}{2D-vector <int>}{Actual shift of the segmentation <SX,SY>}
  \cmdopt{skip}{k}{int}{Remove this number of images from the beginning of the series}
  }
  \item [Example:]Shift the segmentations of a set segement.set by -20 pixels in horizontal direction and -30 
                pixels in vertical direction (as if the images where cropped starting at (20,30) 
                and store it in translate.set. The shiftes files are named translatedXXXX.png
  \begin{lstlisting}
mia-2dsegshift -i segment.set -o translate.set -g translated -S "<20,30>"
  \end{lstlisting}
  \item [See also:] \sa{mia-2dsegshiftperslice}
  \end{description}
  
  LatexEnd
*/


#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cmath>
#include <list>
#include <cassert>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>

#include <mia/core.hh>
#include <mia/2d/SegSet.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/internal/main.hh>

using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;


const SProgramDescrption g_description = {
	"Myocardial Perfusion Analysis", 
	
	"This program move the segmentation(s) of an image series by using a shift "
	"that is equal for all slices. The program also may remove images from the "
	"begin of the series. The program can be used to correct the segmentation "
	"of the images if the images where cropped.", 

	"Shift the segmentations of a set segement.set by -20 pixels in horizontal direction and -30 "
	"pixels in vertical direction (as if the images where cropped starting at (20,30) "
	"and store it in translate.set. The shiftes files are named translatedXXXX.png", 
	
	"-i segment.set -o translate.set -g translated -S \"<20,30>\""
}; 

CSegSet load_segmentation(const string& s)
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(s);
	return CSegSet(*parser.get_document());
}

int do_main(int argc, char *argv[])
{
	string src_filename;
	string out_filename;
	string shift_filename("crop");
	size_t skip = 2;

	C2DFVector shift;

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "input segmentation set", CCmdOption::required));
	options.add(make_opt( shift_filename, "image-file", 'g', "output image filename base"));

	options.add(make_opt(shift, "shift", 'S', "shift of segmentation"));
	options.add(make_opt(skip, "skip", 'k', "skip frames at the begining"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset = load_segmentation(src_filename);

	cvinfo() << "shift by " << shift << ", skip " << skip << " and rename to base " << shift_filename << "\n";
	CSegSet shifted = src_segset.shift_and_rename(skip, shift, shift_filename);

	auto_ptr<xmlpp::Document> outset(shifted.write());

	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();

	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main); 
