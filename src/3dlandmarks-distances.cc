/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#define VSTREAM_DOMAIN "mia-3dlandmark-distance"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/landmarklistio.hh>
#include <mia/3d/transformio.hh>

NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
	{pdi_group,
	 "Registration, Comparison, and Transformation of 3D images"}, 
	 
        {pdi_short,
         "Evaluate per landmark distances between 3D landmark sets."},
        
        {pdi_description,
         "Evaluate Euclidian distances between the corresponding landmarks in two landmark sets. "
         "The programs prints out only values for landmarks that are availabe and have location "
         "values in both sets "}, 
	
        {pdi_example_descr,
         "Evaluate the distance between the landmarks in set input1.lmx and input2.lmx and write "
         "the result to stdout"},
	
	{pdi_example_code,
	 "-1 input1.lmx -2 input2.v"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

        string src1_filename;
	string src2_filename;
        
	const auto& lmxio = C3DLandmarklistIOPluginHandler::instance();

	options.add(make_opt( src1_filename, "in-file-1", 'i', "input landmark set 1", CCmdOptionFlags::required_input, &lmxio));
	options.add(make_opt( src2_filename, "in-file-2", 'o', "input landmark set 2", CCmdOptionFlags::required_input, &lmxio));
	options.set_stdout_is_result();

        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto lms1 = lmxio.load(src1_filename);
	auto lms2 = lmxio.load(src2_filename);

        if (!lms1 || lms1->size() == 0) {
                throw create_exception<invalid_argument>("No landmarks found in '",src1_filename,"'"); 
        }

        if (!lms2 || lms2->size() == 0) {
                throw create_exception<invalid_argument>("No landmarks found in '",src2_filename,"'"); 
        }

        for_each(lms1->begin(), lms1->end(), [&lms2](C3DLandmarklist::value_type& ilm1){
                        if (!ilm1.second->has_location())
                                return;  
                        auto pendant = lms2->get(ilm1.first); 
                        if (!pendant)
                                return; 
                        
                        if (pendant->has_location()) {
                                auto delta = pendant->get_location() - ilm1.second->get_location(); 
                                cout << "'" << ilm1.first << "' " << delta.norm() << "\n"; 
                        }
                });
        
        return EXIT_SUCCESS; 
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
