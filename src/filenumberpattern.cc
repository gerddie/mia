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
  LatexBeginProgramSection{Miscellaneous programs}
  \label{sec:miscprogs}
  
  This is a collection of programs that no not actually work on images but may be 
  related to image processing. 
  LatexEnd
*/

/*
  LatexBeginProgramDescription{Miscellaneous programs}

  \subsection{mia-filenumberpattern}
  \label{mia-filenumberpattern}
  
  \begin{description}
  \item [Description:] This program prints out the number pattern in the numbering of series of 
   files like they are used in the stack image filtering (section \ref{sec:2dstackprograms}). 
   The files must have a pattern like nameXXXXX.ext with XXXXX being digits. 
  The program is called like 
  \
  \begin{lstlisting}
mia-filenumberpattern -i <file name>
  \end{lstlisting}
  and will print out a string of zeros whose length correspond to the number of digits in the file number pattern, 
   i.e. an input 'image000.ext' will result in '000' to be printed.  
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  }

  \item [Example:] Evaluate the number 
   \
  \begin{lstlisting}
mia-filenumberpattern -i image0000.exr
  \end{lstlisting}
  \item [Remark:] It is not required that the given input file actually exists. 
  \end{description}

  LatexEnd
*/



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <iomanip>
#include <string>


#include <mia/internal/main.hh>
#include <mia/core/filetools.hh>
#include <mia/core/cmdlineparser.hh>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
	"Miscellaneous programs", 
	"This program is used to extract a extract the number pattern for "
	"file012312.ext type file names.", 
	NULL, 
	NULL
}; 

int do_main( int argc, char *argv[] )
{
	string in_filename;
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image example name", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	size_t format_width = get_filename_number_pattern_width(in_filename);
	cout << setw(format_width) << setfill('0') << 0;
	return 0;
}

MIA_MAIN(do_main); 
