/* -*- mia-c++ -*- 
 *
 * Copyright (c) Leipzig, Madrid 1999-2011
 *     Gert Wollny <gw.fossdev@gmail.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

/*
  LatexBeginProgramDescription{Miscellaneous programs}
  
  \subsection{mia-labelsort}
  \label{mia-labelsort}

  \begin{description}
  \item [Description:] This program reads a histogram and sorts the number of pixels for each intensity 
    ignoring the intensity 0. Then, the program writes out a map that makes it possible to re-color the
    images so that the intensities with the highest number of pixels have lower intensities. 
    The histogram must be in the format as written by \emph{mia-multihist} \ref{mia-multihist}. 

  The program is called like 
  \
  \begin{lstlisting}
mia-labelsort -i <input histogram>  -o <output mapping> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  }

  \item [Example:] Evaluate read histogram "histo.txt" and write the mapping file "map.txt"
   \
  \begin{lstlisting}
mia-labelsort -i histo.txt  -o map.txt 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <queue>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <mia/core/labelmap.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

NS_MIA_USE; 
using namespace std; 

const char g_description[] = "This program is used create a mapping based on a histogram that "
	  "puts the intensities with high values at the beginning of the histogram. The main "
	  "use case will be to soft labels of connected components in out-of-core image processing.";

typedef pair<double, size_t> CEntry; 

bool operator < (const CEntry& a, const CEntry& b) 
{
	return a.second < b.second; 
}


int do_main(int argc, const char *argv[]) 
{
	string in_filename; 
	string out_filename; 
	CCmdOptionList options(g_description);

	options.add(make_opt( in_filename, "in-file", 'i', "input file name containing the histogram", 
			      CCmdOption::required)); 
	options.add(make_opt( out_filename, "out-file", 'o', "output file name to store probabilities", 
			      CCmdOption::required)); 

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	priority_queue<CEntry> hist; 
	
	ifstream ifs( in_filename); 
	
	vector<double> histo; 
	// read input data

	CEntry entry; 
	while (ifs.good()) {
		float val, cnt;
		ifs >> entry.first >> entry.second; 
		histo.push_back(cnt); 
	}
	ifs.close(); 
		

	cvmsg() << "got a (label) histogram with " << hist.size() << " values\n";
		
	CLabelMap result; 
		
	while (!hist.empty()) {
		CEntry e = hist.top(); 
		hist.pop(); 
		result[e.second] = e.first;
	}

	
	unique_ptr<ostream> os(out_filename == string("-") ? &cout : new ofstream(out_filename)); 
	result.save(*os); 
	return os->good() ? EXIT_SUCCESS : EXIT_FAILURE; 
}

int main(int argc, const char *args[]) 
{
	try {
		return do_main(argc, args); 
	}
	catch (const runtime_error &e){
		cerr << args[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << args[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << args[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << args[0] << " unknown exception" << endl;
	}
	return EXIT_FAILURE;	
}
