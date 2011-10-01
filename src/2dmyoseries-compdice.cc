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

  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dsegseriesstats}
  \label{mia-2dsegseriesstats}

  \begin{description} 
  \item [Description:] 
        This program is used evaluate the dice index for each frame of a segmentation set 
	with reference to a specific frame. The result is printed to stdout. 

  The program is called like 
  \begin{lstlisting}
mia-2dsegseriesstats -i <segmenation set> -r <reference frame> 
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{original}{i}{string}{segmentation set}
  \cmdopt{reference}{r}{int}{Reference frame to base evaluation on.}
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  }

  \item [Example:]Evaluate the dice index of segmentation set \emph{segment.set} with reference 30 and 
                  skipping the first two frames. 
  \begin{lstlisting}
mia-2dsegseriesstats -i segment.set -r 30 -k 2
  \end{lstlisting}
  \item [Remark:] This program is used to validate motion compensation algorithms. 
  \item [See also:] \sa{mia-2dmyomilles}, \sa{mia-2dmyoserial-nonrigid}, \sa{mia-2dmyoperiodic-nonrigid}, 
                    \sa{mia-2dmyoica-nonrigid}, \sa{mia-2dmyopgt-nonrigid}
  \end{description}
  
  LatexEnd
*/



#define VSTREAM_DOMAIN "2dmyoseries-dice"

#include <libxml++/libxml++.h>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <ostream>
#include <fstream>

using xmlpp::DomParser;

using namespace mia; 
using namespace std; 


const char *g_description = 
	"This program is used to evaluate the per-frame dice index of \n"
	"segmented regions of two image series"; 


float dice_value(const C2DUBImage& mask1, const C2DUBImage& mask2) 
{
	assert (mask1.get_size() == mask2.get_size()); 
	int schnitt = 0; 
	int sum = 0; 
	
	for (auto m1 = mask1.begin(), m2 = mask2.begin(); 
	     m1 != mask1.end(); ++m1, ++m2) {
	
		if (*m1) 
			++sum; 
		if (*m2) 
			++sum; 
		
		if (*m1 && *m2)
			++schnitt; 
	}
	
	return sum ? (2.0f * schnitt) / sum : 1.0f; 
}

int do_main( int argc, const char *argv[] )
{
	string org_filename;
	string ref_filename;
	size_t skip = 2; 

	CCmdOptionList options(g_description);
	options.add(make_opt( org_filename, "first", '1', "first segmentation set", CCmdOption::required));
	options.add(make_opt( ref_filename, "second", '2', "second segmentation set", CCmdOption::required));
	options.add(make_opt( skip, "skip", 'k', "images to skip atthe begin of the series")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	CSegSetWithImages original(org_filename, true); 
	CSegSetWithImages ref(ref_filename, true); 

	auto original_frames = original.get_frames(); 
	auto reference_frames = ref.get_frames(); 

	if (original_frames.size() != original_frames.size())
		throw invalid_argument("Both series must contain the same number of frames"); 
	
	if (skip  >= original_frames.size())
		THROW(invalid_argument, "skip ("<< skip <<") must be smaller then number of availabe frames("
		      << original_frames.size() << ")"); 
	


	for (size_t i = skip; i < original_frames.size(); ++i)  {
		auto o = original_frames[i].get_section_masks(1);
		auto r = reference_frames[i].get_section_masks(1);
		cout << dice_value(o, r) << '\n'; 
	}
	
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
