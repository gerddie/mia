/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/2d.hh>
#include <sstream>
#include <iomanip>
#include <mia/2d/multicost.hh>
#include <mia/internal/main.hh>

NS_MIA_USE
using namespace boost;
using namespace std;


const SProgramDescription g_description = {
	{pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 
	{pdi_short, "Evaluate the similarity between two 2D images."}, 
	{pdi_description, 	"This program is used to evaluate the cost between two "
	 "images by using a given cost function."}, 
	{pdi_example_descr, "Evaluate the SSD cost function between image1.png and image2.png"}, 
	{pdi_example_code, "image:src=image1.png,ref=image2.png,cost=ssd"}

}; 

// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);

	if (options.parse(argc, argv, "cost", &C2DFullCostPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cvfatal() << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}

	C2DFullCostList cost_list;
	for(auto i = cost_chain.begin(); i != cost_chain.end(); ++i) {
		cost_list.push(produce_2dfullcost(*i)); 
	}
	auto size = C2DBounds::_0; 
	cost_list.reinit(); 
	cost_list.get_full_size(size); 
	cost_list.set_size(size); 
	cout << cost_list.cost_value() << "\n";

	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
