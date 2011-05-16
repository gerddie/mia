
/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dsegshiftperslice}
  \label{mia-2dsegshiftperslice}

  \begin{description} 
  \item [Description:] 
	This program move the segmentation(s) of an image series by using a shift 
          that is given on a per-slice base. 
        The program can be used to correct the segmentation of the images if 
    	  a linear registration was executed that only applies a translation and 
          does not correct the segmentation automatically. 

  The program is called like 
  \begin{lstlisting}
mia-2dsegshiftperslice -i <input set> -o <output set> \
                       -g <out image basename> -S <shift file basename>
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{image-file}{g}{}{base name of the output image file. The numbering scheme and the file type of the 
                           input images will be preserved.These files will not be touched and 
                             should actually be the result of translation based linear registration.}
  \cmdopt{shift}{S}{stringh}{File name base for the files containing the shift for each individual slice. 
                             The file numbering scheme must correspond.}
			     }
  \item [Example:]Shift the segmentations of a set segement.set and store it in translate.set. The 
                  shifts are stored in files "shiftXXXX.txt" and the registered files are named translatedXXXX.png
  \begin{lstlisting}
mia-2dsegshiftperslice -i segment.set -o translate.set -g translated -S shift 
  \end{lstlisting}
  \item [Remark:] Obsolete. Use \sa{mia-2dmyomilles} to achive registration 
                  and correct the segmentation. 
  \item [See also:] \sa{mia-2dsegshift}
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
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/SegSet.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>





using namespace std;
using namespace mia;
using xmlpp::DomParser;
namespace bfs=boost::filesystem;


CSegSet load_segmentation(const string& s)
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(s);
	return CSegSet(*parser.get_document());
}

static string get_number(const string& fname)
{
	bfs::path f(fname);
	string the_stem = __bfs_get_stem(f);
	auto rs = the_stem.rbegin();
	string result;
	while (rs != the_stem.rend() && isdigit(*rs))
		result.insert(0,1,*rs++);
	return result;
}

const char *g_description = 
	"This program is used to shift the 2d segmentation of a segmentation set individually "
	"on a per slice base." 
	;

int do_main(int argc, const char *argv[])
{
	string src_filename;
	string out_filename;
	string shift_filename("crop");

	string shift_value_filebase("shift");

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output segmentation set", CCmdOption::required));
	options.add(make_opt( shift_filename, "image-file", 'g', "output image filename base"));

	options.add(make_opt( shift_value_filebase, "shift", 'S', "shift of segmentation - base name ", 
				    CCmdOption::required));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSet src_segset = load_segmentation(src_filename);
	CSegSet::Frames& frames = src_segset.get_frames();

	for (auto i = frames.begin(); i != frames.end(); ++i) {
		string nr = get_number(i->get_imagename());
		stringstream shift_file_name;
		shift_file_name << shift_value_filebase << nr << ".txt";
		ifstream shift_file(shift_file_name.str());
		C2DFVector shift;
		shift_file >> shift;
		i->shift(shift, i->get_imagename());
	}


	auto_ptr<xmlpp::Document> outset(src_segset.write());

	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();

	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, const char *argv[] )
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


