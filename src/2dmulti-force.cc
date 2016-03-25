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

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/minmax_element.hpp>
#include <mia/2d/transformfactory.hh>

NS_MIA_USE
using namespace boost;
using namespace std;


const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 2D images"}, 
	{pdi_short, "Registration force between two images"}, 
	{pdi_description, "This program evaluates the 2D image cost force norm image of a given cost function set. "
	 "The input images must be of the same dimensions and gray scale (whatever bit-depth)."}, 
	{pdi_example_descr, "Evaluate the force normimage weighted sum of costs SSD and NGF of "
	 "image1.v and image2.v. and store the result to force.v."}, 
	{pdi_example_code, "-o force.v \n image:cost=ssd,src=image1.v,ref=image2.v,weight=0.1\n "
	 "image:cost=ngf,src=image1.v,ref=image2.v,weight=2.0"}
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

	const auto& imageio = C2DImageIOPluginHandler::instance();
	const auto& costcreator = C2DFullCostPluginHandler::instance(); 

	options.add(make_opt( out_filename, "out-file", 'o', "output norm image", CCmdOptionFlags::required_output, &imageio));

	if (options.parse(argc, argv, "cost", &costcreator) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto cost_chain = options.get_remaining();

	if (cost_chain.empty()) {
		cerr << "require cost functions given as extra parameters\n";
		return EXIT_FAILURE;
	}


	C2DFullCostList cost_list;
	for(auto i = cost_chain.begin(); i != cost_chain.end(); ++i) {
		auto c = costcreator.produce(*i);
		assert(c); 
		cost_list.push(c);
	}

	cost_list.reinit(); 
	C2DBounds size; 
	if (!cost_list.get_full_size(size)) {
		throw invalid_argument("Input images given for the cost functions are no of the same size"); 
	}
	cost_list.set_size(size); 
	
	if ( out_filename.empty()) {
		cout <<  cost_list.cost_value() << endl;
		return EXIT_SUCCESS;
	}
	
	auto tff = C2DTransformCreatorHandler::instance().produce("vf:imgkernel=[bspline:d=0],imgboundary=zero"); 
	auto t = tff->create(size); 
	auto params = t->get_parameters(); 
	std::fill(params.begin(), params.end(), 0.0); 
	t->set_parameters(params); 

	cost_list.evaluate(*t, params);

	C2DFImage *result = new C2DFImage(size);
	int i = 0; 
	
	for (auto ir = result->begin(); ir != result->end(); ++ir, i+=2){
		*ir = sqrt(params[i] * params[i] + params[i+1] * params[i+1]); 
	}

	P2DImage norm_img(result);

	const C2DImageIOPlugin::PixelTypeSet& pts = imageio.preferred_plugin(out_filename).supported_pixel_types();

	// convert optimized to ubyte
	if (pts.find(it_float) == pts.end())
		norm_img = C2DFilterPluginHandler::instance().produce("convert")->filter(*norm_img);

	if ( !save_image(out_filename, norm_img) )
		throw runtime_error(string("unable to save to: ") + out_filename);

	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
