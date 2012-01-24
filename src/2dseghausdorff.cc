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
  
  \subsection{mia-2dseghausdorff}
  \label{mia-2dseghausdorff}

  \begin{description} 
  \item [Description:] 
  Get the per-slice Hausdorff distance of a segmentation with respect to a given reference frame 
     of the same segmentation set.

  The program is called like 
  \begin{lstlisting}
mia-2dseghausdorff -i <segmentation set> -r <reference frame> 
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinset
  \cmdopt{ref-frame}{r}{int}{refernce frame to compare the segmentation to}
  }

  \item [Example:]Evaluate the Hausdorff distance of the segmentations of set segment.set with 
                  respect to the segmentation given in frame 20. 
  \begin{lstlisting}
mia-2dseghausdorff -i segment.set -r 20 
  \end{lstlisting}
  \item [See also:] \sa{mia-2dsegcompare}
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

const SProgramDescription g_description = {
	"Myocardial Perfusion Analysis", 

	"Get the per-slice Hausdorff distance of a segmentation with respect to a given reference frame "
	"and print it to stdout.",
	
	"Evaluate the Hausdorff distance of the segmentations of set segment.set with "
	"respect to the segmentation given in frame 20.",
	
	" -i segment.set -r 20"
}; 

int do_main(int argc, char *argv[])
{
	string src_filename;
	size_t reference = 0;
	int skip = 0; 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( reference, "ref-frame", 'r', "reference frame", CCmdOption::required));
	options.add(make_opt( skip, "skip", 'k', "skip frames at the beginning"));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(src_filename);

	if (!parser)
		throw runtime_error(string("Unable to parse input file:") + src_filename);

	CSegSet segset(*parser.get_document());

	if (skip < 0) {
                // if RV peak is given in the segmentation file, use it, otherwiese use 
		// absolue value of skip 
		int sk = segset.get_RV_peak(); 
		skip = (sk < 0 ) ? -skip : sk; 
	}


	const CSegSet::Frames& frames = segset.get_frames();

	if (reference >= frames.size())
		throw invalid_argument("Reference frame outside range");

	if (skip >= static_cast<long>(frames.size()))
		throw invalid_argument("Can't skip the whole series");


	const CSegFrame& ref = segset.get_frames()[reference];

	CSegSet::Frames::const_iterator iframe = segset.get_frames().begin() + skip;
	CSegSet::Frames::const_iterator eframe = segset.get_frames().end();

	size_t i = 0;
	while (iframe != eframe) {
		if (i != reference) {
			cout << ref.get_hausdorff_distance(*iframe) << "\n";
		}else{
			cout << "0\n";
		}
		++i;
		++iframe;
	}
	return 0;

}

MIA_MAIN(do_main); 
