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
  
  \begin{description}
  \item [Program:] \emph{mia-3dcost}
  \hrule 
  \item [Description:] This program evauates the cost function as given ba the command line 
  by using the plug-ins provided through (section \ref{sec:fatcost3d}). 
  \lstset{language=bash}
  \begin{lstlisting}
mia-3dcost  <cost1> [<cost2>] ... 
  \end{lstlisting}
  \item [Options:] $\:$
  
  The Cost function(s) is(are) given as free parameters.  

  \item [Example:] Evaluate the weigtes sum of SSD and NGF between image1.v and image2.v 
   \lstset{language=bash}
  \begin{lstlisting}
mia-3dcost ssd:src=image1.v,ref=image2.v,weight=0.1 \
           ngf:src=image1.v,ref=image2.v,weight=1.0 
  \end{lstlisting}
  \item [Remark:] The used plug-ins should be replaced by fullcost (section \ref{sec:fullcost3d}). 
  \end{description}

  LatexEnd
*/


#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <mia/3d/fatcost.hh>


static const char *program_info = 
	"This program is used to evaluate the combinde cost value as given.\n"
	"Basic usage:\n"
	"  mia-3dcost  <cost plugin> [<cost plugin>] ...\n"; 

NS_MIA_USE
using namespace boost;
using namespace std;

// set op the command line parameters and run the registration
int do_main(int argc, const char **argv)
{

	CCmdOptionList options(program_info);

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	

	vector<const char *> cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cvfatal() << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}


	C3DImageFatCostList cost_list;
	for(vector<const char *>::const_iterator i = cost_chain.begin(); i != cost_chain.end(); ++i) {
		P3DImageFatCost c = C3DFatImageCostPluginHandler::instance().produce(*i);
		if (c)
			cost_list.push_back(c);
	}
	if (cost_list.empty()) {
		cerr << "Could not create a single cost function\n";
		return EXIT_FAILURE;
	}
	cout << cost_list.value() << "\n";

	return EXIT_SUCCESS;
}

// for readablility the real main function encapsulates the do_main in a try-catch block
int main(int argc, const char **argv)
{
	try {
		return do_main(argc, argv);
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n";
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n";
	}
	catch (std::exception& err) {
		cerr << "exception: " << err.what() << "\n";
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE;
}
