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


#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_USE;
using namespace std; 

const SProgramDescription g_description = {
	"Miscellaneous programs", 

	"Evaluate the norm image of a 3D vector field.", 
	
	"This program converts a 3d vector field to an image of its voxel-wise norm.", 

	"Evaluate the norm image norm.v from a vector field field.v.", 
	
	"-i field.v -o norm.v"
}; 

struct FVector2Norm {
	float operator ()(const C3DFVector& v) const {
		return v.norm(); 
	}
}; 

int do_main(int argc, char *argv[])
{
	string src_filename;
	string out_filename;
	
	CCmdOptionList options(g_description);
	
	options.add(make_opt( src_filename, "in", 'i', "input vector field", CCmdOption::required));
	options.add(make_opt( out_filename, "out", 'o', "output image", CCmdOption::required));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto vf = C3DVFIOPluginHandler::instance().load(src_filename);
	
	C3DFImage *out_image = new C3DFImage(vf->get_size()); 
	P3DImage result(out_image); 
	FVector2Norm to_norm; 
	transform(vf->begin(), vf->end(), out_image->begin(), to_norm); 

	return save_image(out_filename, result) ? 0 : 1; 
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
