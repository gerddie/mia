/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#define VSTREAM_DOMAIN "mia-3dtransform"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/vfio.hh>
#include <mia/internal/main.hh>
#include <iostream>

NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
	{pdi_group, "Registration, Comparison, and Transformation of 3D images" }, 
	{pdi_short, "Compare two Vectorfields."}, 
	{pdi_description, "Compare two vector fields and print the out the difference norm per pixel to cout it it is larger than delta."}, 
	{pdi_example_descr, "Compare vector fields vf1.v and vf2.v and print out all differences above 0.01."}, 
	{pdi_example_code, "-1 vf1.v -2 vf2.v -d 0.01"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string vf1_filename;
	string vf2_filename;
	float delta = 0.0; 

	const auto& vfio =C3DVFIOPluginHandler::instance(); 

	options.add(make_opt( vf1_filename, "in-file-1", '1', "input vector field 1", CCmdOptionFlags::required_input, &vfio));
	options.add(make_opt( vf2_filename, "in-file-2", '2', "input vector field 2", CCmdOptionFlags::required_input, &vfio));
	options.add(make_opt( delta, "delta", 'd', "Maximum difference between vector to be ignored"));
	options.set_stdout_is_result();
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	auto vf1 = vfio.load(vf1_filename); 
	auto vf2 = vfio.load(vf2_filename); 

	if (vf1->get_size() != vf2->get_size()) {
		cverr() << "Field sizes are different: vf1=" << vf1->get_size()
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
			cout << "At " << ivf1.pos()
			     << ", vf1=" << *ivf1 
			     << ", vf2=" << *ivf2 
			     << ", delta = " << *ivf1 - *ivf2 << " (" << d << ")\n"; 
		}
		++ivf1; 
		++ivf2; 
	}
	std::cout  << (diff ? "0" : "1") << "\n"; 
	return diff ? EXIT_FAILURE : EXIT_SUCCESS;	
}

MIA_MAIN(do_main);
