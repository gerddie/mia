/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#define VSTREAM_DOMAIN "mia-3dlandmark-transform"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/landmarklistio.hh>
#include <mia/3d/transformio.hh>

NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
	{pdi_group,
	 "Registration, Comparison, and Transformation of 3D images"}, 
	 
        {pdi_short,
         "Transform 3D landmark positions."},
        
        {pdi_description,
         "Transform the locations of the landmarks by means of a given 3D transformation. "
         "Note, landmark transformations do the inverse of an image transformation (in a "
         "manner of speaking), i.e. given a transformation V(x) in an image transformation "
         "the pixel intensity at x is set to the original intensity at V(x), while a "
         "landmark at x is moved to V(x)."}, 
	
        {pdi_example_descr,
         "Transform the landmarks given in input.lmx by applyning the transfromation stored in trans.v "
         "and store the result in output.lmx"},
	
	{pdi_example_code,
	 "-i input.lmx -t trans.v  -o output.lmx"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

        string src_filename;
	string out_filename;
	string trans_filename;
        
	const auto& lmxio = C3DLandmarklistIOPluginHandler::instance();
	const auto& transio = C3DTransformationIOPluginHandler::instance(); 

	options.add(make_opt( src_filename, "in-file", 'i', "input landmark set", CCmdOptionFlags::required_input, &lmxio));
	options.add(make_opt( out_filename, "out-file", 'o', "output landmark set", CCmdOptionFlags::required_output, &lmxio));
	options.add(make_opt( trans_filename, "transformation", 't', "transformation file", CCmdOptionFlags::required_input, &transio));


        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto transformation = transio.load(trans_filename);
	auto source = lmxio.load(src_filename);

        if (!source || source->size() == 0) {
                throw create_exception<invalid_argument>("No landmarks found in '",src_filename,"'"); 
        }

        if (!transformation) {
                throw create_exception<invalid_argument>("No transformation found in '", trans_filename, "'");
	}

        C3DLandmarklist result(*source); 
        const C3DTransformation& t = *transformation; 

        for_each(result.begin(), result.end(), [&t](C3DLandmarklist::value_type& ilm){
                        C3DLandmark& lm= *ilm.second; 
                        if (lm.has_location())
                                lm.set_location(t(lm.get_location())); 
                }); 
        
        if (!lmxio.save(out_filename,  result)) 
                throw create_exception<runtime_error>("Unable to save landmarks to '", out_filename, "'");
        
        return EXIT_SUCCESS; 
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)


