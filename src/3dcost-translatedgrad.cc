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

#define VSTREAM_DOMAIN "mia-3dcost-translatedgrad"
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/internal/main.hh>


NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Evaluate the cost gradient between two images and convert it to a spline representation."}, 
	{pdi_description, "Evaluate the cost gradient between two images and evaluate the "
	 "transformation related gradient for it based on the given transformation model."}, 
	{pdi_example_descr, "Evaluate the SSD cost between src.v and ref.v and store the "
	 "gradient gradient corresponding to a spline transformation in grad.v3dt."}, 
	{pdi_example_code, "-i src.v -o grad.v3dt -f spline:rate=8 -c ssd"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string src_filename;
	string ref_filename;
	string out_filename;

	string grad_image_filename;
	string cost_grad_filename;

	

	P3DImageCost cost; 
	P3DTransformationFactory transform_creator; 
	
	
	options.add(make_opt( src_filename, "in-file", 'i', "input image ", CCmdOption::required));
	options.add(make_opt( ref_filename, "ref-file", 'r', "reference image ", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output vector field ", CCmdOption::required));
	options.add(make_opt( grad_image_filename, "gradimg-file", 'g', "norm image of the spline transformed gradient"));
	options.add(make_opt( cost_grad_filename, "cost-gradimg-file", 'C', "norm image of the cost gradient"));

	options.add(make_opt( transform_creator, "spline:rate=5", "transForm", 'f', "Transformation the gradient relates to"));
	options.add(make_opt( cost, "ssd", "cost", 'c', "cost function to use"));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	typedef C3DImageIOPluginHandler::Instance::PData PImageVector;

	auto source = load_image<P3DImage>(src_filename);
	auto ref    = load_image<P3DImage>(ref_filename);

	if (!source) {
		throw create_exception<runtime_error>("No image found in '", src_filename, "'");
	}
	if (!ref)
		throw create_exception<runtime_error>("No image found in '", src_filename, "'");

	C3DFVectorfield forcefield(source->get_size());
	cost->set_reference(*ref);  
	cost->evaluate_force(*source, 1.0, forcefield);

	auto t = transform_creator->create(forcefield.get_size()); 
	auto grad = t->get_parameters(); 
	
	t->translate(forcefield, grad);
	t->set_parameters(grad); 
	if (!C3DTransformationIOPluginHandler::instance().save(out_filename, *t)) 
		throw create_exception<runtime_error>("Grad can not be saved to  '", out_filename, "'");

	if (!cost_grad_filename.empty()) {
		C3DFImage image(forcefield.get_size()); 
		float maxnorm = 0.0; 
		transform(forcefield.begin(), forcefield.end(), image.begin(), 
			  [&maxnorm](const C3DFVector& x)->float{
				  float n = x.norm();
				  if (maxnorm < n)
					  maxnorm = n; 
				  return n; 
			  }); 
//		float imn = 1.0/ maxnorm; 
//		transform(image.begin(), image.end(), image.begin(), [imn](float x) {return imn * x;}); 
		save_image(cost_grad_filename, image); 

		cvmsg() << "max gradient norm before translation = " << maxnorm << "\n"; 
	}

	if (!grad_image_filename.empty()) {
		C3DFImage image(forcefield.get_size()); 

		float maxnorm = 0.0; 
		auto ti = t->begin();
		auto te = t->end(); 
		auto ii = image.begin_range(C3DBounds::_0, forcefield.get_size()); 
		
		while ( ti != te )  { 
			const C3DFVector d = *ti - C3DFVector(ii.pos()); 
			*ii = d.norm(); 
			if (maxnorm < *ii) 
				maxnorm = *ii; 
			++ii; 
			++ti; 
		}
//		float imn = 1.0/ maxnorm; 
//		transform(image.begin(), image.end(), image.begin(), [imn](float x) {return imn * x;}); 
		save_image(grad_image_filename, image); 
		cvmsg() << "max gradient norm after translation = " << maxnorm << "\n"; 
		
	}
	
	return EXIT_SUCCESS;	
}

MIA_MAIN(do_main);
