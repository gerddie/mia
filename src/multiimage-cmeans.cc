/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdexcept>
#include <cmath>
#include <cassert>
#include <numeric>
#include <fstream>


#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>


const SProgramDescription g_description = {
	{pdi_group,"Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short,"C-means of intensities of an image series."}, 
	{pdi_description, "This programs reads a series of images and does a c-means "
         "classification of the intensities. The output is a map from intensity ranges"
         " to the class probabilities like follows:\n"
         "  min max prob1 prob2 prob3 ...\n"}, 
	{pdi_example_descr, "Read al images from the series imageXXXX.exr, classify into 3 classes, "
         "and save the result into probmap.txt."}, 
	{pdi_example_code, "-i image0000.exr -o probmap.txt --classes 3"}
}; 


class CMeansParameterTranslater : public CValueTranslator {
        
}; 

int do_main(int argc, char *argv[])
{
	string in_filename;
        string out_filename;

        unsigned nclasses = 0; 
        unsigned max_iter = 100; 
        
        string class_centers = "even"; 
        

        const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
        CCmdOptionList options(g_description);
        
	options.set_group("File-IO"); 
        options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be read from",
                              CCmdOptionFlags::required_input, &imageio));
        options.add(make_opt( in_filename, "out-file", 'o', "output map to write the class probability map to",
                              CCmdOptionFlags::required_input, &imageio));
        
        
        options.add(make_opt( nclasses, "nclasses", 'n', "number of classes to partition into")); 
	options.add(make_opt( max_iter, "max-iter", 'm', "maximum number of iterations")); 
	options.add(make_opt( class_centers, "class-centers", 'c', "initial class centers")); 

        
        
        

}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 




