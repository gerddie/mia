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

#define VSTREAM_DOMAIN "mia-3dtransform"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/internal/main.hh>


NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
	"Registration, Comparison, and Transformation of 3D images", 
	"Compare two Vectorfields.", 

	"Compare two vector fields and print out the difference norm per pixel.", 
	
	"Compare vector fields vf1.v and vf2.v and print out all differences above 0.01.",
	
	"-1 vf1.v -2 vf2.v -d 0.01"
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string vf1_filename;
	string vf2_filename;
	float delta = 0.0; 


	options.add(make_opt( vf1_filename, "in-file-1", '1', "input vector field 1", CCmdOption::required));
	options.add(make_opt( vf2_filename, "in-file-2", '2', "input vector field 2", CCmdOption::required));
	options.add(make_opt( delta, "delta", 'd', "Maximum difference between vector to be ignored"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	auto vf1 = C3DVFIOPluginHandler::instance().load(vf1_filename); 
	auto vf2 = C3DVFIOPluginHandler::instance().load(vf2_filename); 

	if (vf1->get_size() != vf2->get_size()) {
		cvfail() << "Field sizes are different: vf1=" << vf1->get_size()
			 << ", vf2="<< vf2->get_size() << "\n"; 
		return EXIT_FAILURE;	
	}

	auto ivf1 = vf1->begin_range(C3DBounds::_0, vf1->get_size()); 
	auto evf1 = vf1->end_range(C3DBounds::_0, vf1->get_size()); 
	auto ivf2 = vf2->begin(); 

	bool diff = false; 
	while (ivf1 != evf1) {
		
		auto d = (*ivf1 - *ivf2).norm(); 
		if (d > delta) {
			diff = true; 
			cvfail() << "At " << ivf1.pos()
				 << ", vf1=" << *ivf1 
				 << ", vf2=" << *ivf2 
				 << ", delta = " << *ivf1 - *ivf2 << " (" << d << ")\n"; 
		}
		++ivf1; 
		++ivf2; 
	}
	return diff ? EXIT_FAILURE : EXIT_SUCCESS;	
}

MIA_MAIN(do_main);
