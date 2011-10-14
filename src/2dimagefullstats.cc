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

  \subsection{mia-2dimagestats}
  \label{mia-2dimagestats}
  
  \begin{description}
  \item [Description:] This progranm is used to evaluate some statistics of an image.
	Output is Mean, Variation, Median, Min and Max of the intensity values.
  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagestats -i <file name>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  }

  \item [Example:] Evaluate the statistics of image input.v.  
   \
  \begin{lstlisting}
mia-2dimagestats -i input.v 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/


#include <sstream>
#include <mia/internal/main.hh>
#include <mia/core/fullstats.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/2dimageio.hh>


NS_MIA_USE;
using namespace std;


const SProgramDescrption g_general_help = {
	"2D image processing", 
	
	"This progranm is used to evaluate some statistics of an image. " 
	"Output is Mean, Variation, Median, Min and Max of the intensity values.", 
	
	"Evaluate the statistics of image input.png", 
	
	"-i input.png"
}; 


class CStatsEvaluator : public TFilter<CFullStats> {
public:
	template <typename T>
	CFullStats operator () (const T2DImage<T>& image) const {
		return CFullStats(image.begin(), image.end());
	}
};

int do_main( int argc, char *argv[] )
{

	string in_filename;
	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image", CCmdOption::required));
	
	if (options.parse(argc, argv, "cost") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	
	
	// read image
	P2DImage image = load_image2d(in_filename);
	
	
	CFullStats stats = mia::filter(CStatsEvaluator(), *image);
	cout << stats << "\n";
	
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 
