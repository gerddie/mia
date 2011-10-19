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
  This program implements 2D image cost force evaluation.
  Depending on the available plug-ins it can accomodate various models and cost-functions.
  So far input images can be given as PNG, TIF, BMP and OpenEXR (depending on the installed plug-ins)
  The input images must be of the same dimensions and gray scale (whatever bit-depth).
  The vector field will be saved as a EXR image with two frames of float values, one for the X
  and one for the Y values of the vectors.
  Other vector field output plug-ins might be supported depending on the installed plug-ins.
*/

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/minmax_element.hpp>

NS_MIA_USE
using namespace boost;
using namespace std;


const SProgramDescrption g_description = {
	"Miscellaneous", 
	
	"This program evaluates the 2D image cost force norm image of a given cost function set. "
	"The input images must be of the same dimensions and gray scale (whatever bit-depth). ", 
	
	"Evaluate the force normimage weighted sum of costs SSD and NGF of image1.v and image2.v. and store the "
	"result to force.v.",

	"-o force.v ssd:src=image1.v,ref=image2.v,weight=0.1 ngf:src=image1.v,ref=image2.v,weight=2.0"
}; 

struct FGetNorm  {
	float operator ()(const C2DFVector& x) const {
		return x.norm();
	}
};


// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);
	string out_filename;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	options.add(make_opt( out_filename, "out-file", 'o', "output norm image", CCmdOption::required));

	if (options.parse(argc, argv, "cost") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	vector<const char *> cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cerr << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}

	C2DImageFatCostList cost_list;
	for(vector<const char *>::const_iterator i = cost_chain.begin(); i != cost_chain.end(); ++i) {
		P2DImageFatCost c = C2DFatImageCostPluginHandler::instance().produce(*i);
		if (c)
			cost_list.push_back(c);
	}
	if (cost_list.empty()) {
		cerr << "Could not create a single cost function\n";
		return EXIT_FAILURE;
	}


	C2DFVectorfield force(cost_list.get_size());

	if ( out_filename.empty()) {
		cout <<  cost_list.value() << endl;
		return EXIT_SUCCESS;
	}

	C2DFImage *result = new C2DFImage(force.get_size());

	transform(force.begin(), force.end(), result->begin(), FGetNorm());

	P2DImage norm_img(result);

	const C2DImageIOPlugin::PixelTypeSet& pts = imageio.preferred_plugin(out_filename).supported_pixel_types();

	// convert optimized to ubyte
	if (pts.find(it_float) == pts.end())
		norm_img = C2DFilterPluginHandler::instance().produce("convert")->filter(*norm_img);

	C2DImageIOPluginHandler::Instance::Data images;
	images.push_back(norm_img);


	if ( !imageio.save(out_filename, images) )
		throw runtime_error(string("unable to save to: ") + out_filename);

	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
