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

#include <mia/core.hh>
#include <mia/3d.hh>
#include <sstream>
#include <iomanip>
#include <mia/3d/fatcost.hh>

NS_MIA_USE
using namespace std;
using namespace boost;


const SProgramDescription g_description = {
	{pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Evaluate similarity of two 3D images."}, 
	{pdi_description,
	 "This program evauates the cost function as given by the free parameters on the command line."}, 
	
}; 

// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);

	if (options.parse(argc, argv, "cost", &C3DFatImageCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	

	auto cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cvfatal() << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}


	C3DImageFatCostList cost_list;
	for(auto i = cost_chain.begin(); i != cost_chain.end(); ++i) {
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
