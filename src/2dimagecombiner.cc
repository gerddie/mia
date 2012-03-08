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

#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;
using boost::any_cast; 


const SProgramDescription g_description = {
	"Analysis, filtering, combining, and segmentation of 2D images", 
	
	"Combine two image by a given operation.", 
	
	"Combine image A.exr and image B.exr  by adding the intensity values and save the output to sum.exr.", 
	
	"-i A.exr -r B.exr -o sum.exr -p add"
}; 

int do_main( int argc, char *argv[] )
{

	string in1_filename;
	string in2_filename;
	string out_filename;

	C2DImageCombinerPluginHandler::ProductPtr combiner;

	stringstream combiner_names;

	CCmdOptionList options(g_description);
	options.add(make_opt( in1_filename, "in-file-1", '1', 
				    "first input image to be combined", CCmdOption::required));
	options.add(make_opt( in2_filename, "in-file-2", '2', 
				    "second input image to be combined", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.add(make_opt( combiner, "absdiff", "operation", 'p', "operation to be applied"));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

		

	//CHistory::instance().append(argv[0], "unknown", options);

	auto image1 = load_image2d(in1_filename); 
	auto image2 = load_image2d(in2_filename); 

	auto output = combiner->combine(*image1, *image2); 
	if (!save_image(out_filename, output)) 
		THROW(runtime_error, "unable to save result in '" << out_filename << "'");
	
	return EXIT_SUCCESS;
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 


