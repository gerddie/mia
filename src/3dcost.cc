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

  \subsection{mia-3dcost}
  \label{mia-3dcost}
  
  \begin{description}
  \item [Description:] This program evauates the cost function as given ba the command line 
  by using the plug-ins provided through (section \ref{sec:fatcost3d}). 
  \
  \begin{lstlisting}
mia-3dcost  <cost1> [<cost2>] ... 
  \end{lstlisting}
  \item [Options:] $\:$
  
  The Cost function(s) is(are) given as free parameters.  

  \item [Example:] Evaluate the weigtes sum of SSD and NGF between image1.v and image2.v 
   \
  \begin{lstlisting}
mia-3dcost ssd:src=image1.v,ref=image2.v,weight=0.1 \
           ngf:src=image1.v,ref=image2.v,weight=1.0 
  \end{lstlisting}
  \item [Remark:] The used plug-ins should be replaced by fullcost (section \ref{sec:3dfullcost}).
  \end{description}

  LatexEnd
*/


#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <mia/3d/fatcost.hh>

NS_MIA_USE
using namespace std;
using namespace boost;


const SProgramDescrption g_description = {
	"Miscellaneous programs", 
	
	"This program evauates the cost function as given on the command line.", 

	NULL, NULL
}; 

// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);

	if (options.parse(argc, argv, "cost", &C3DFatImageCostPluginHandler::instance()) != CCmdOptionList::hr_no)
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

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
