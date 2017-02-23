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

#include <mia/core/cmdlineparser.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/vfio.hh>
#include <mia/internal/main.hh>


NS_MIA_USE
using namespace std;

const SProgramDescription g_description = {
	{pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Convert a 3D vectorfield from a 3D transformation."}, 

	{pdi_description, "Creates a 3D transformation from a vector field. The input vector field "
	 "is simply encapsulated into the transformation file format. The boundary "
	 "conditions and the image interpolator kernel can be set at the command line."}, 
	
	{pdi_example_descr, "Transform the input vector field field.vtk to the transformation trans.v3df and "
	 "specify zero boundary conditions and b-splines of degree 2 for image interpolation.",}, 
	
	{pdi_example_code, "-i field.vtk -o trans.v3df --imgkernel bspline:d=2 --imgboundary zero"}
}; 

int do_main(int argc, char **argv)
{
	CCmdOptionList options(g_description);

	string src_filename;
	string out_filename;
	
	PSplineKernel image_interpolator; 
	PSplineBoundaryCondition image_boundary; 

	options.add(make_opt( src_filename, "in-file", 'i', "input transformation ", 
			      CCmdOptionFlags::required_input, &C3DVFIOPluginHandler::instance()));
	options.add(make_opt( out_filename, "out-file", 'o', "output vector field ", 
			      CCmdOptionFlags::required_output, &C3DTransformationIOPluginHandler::instance()));

	options.add(make_opt( image_interpolator, "bspline:d=3", "imgkernel", 'k', "image interpolator kernel which is "
			      "used when the transformation is applied to an image"));
	options.add(make_opt( image_boundary, "mirror", "imgboundary", 'b', "Boundary condition applied "
			      "when the transformation is used to transform an image"));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto vf = C3DVFIOPluginHandler::instance().load(src_filename); 
	
	stringstream transform_decr; 
	transform_decr << "vf:imgkernel=[" << image_interpolator->get_init_string() <<"]," 
		       << "imgboundary=[" << image_boundary->get_init_string() <<"]"; 
	
	auto vftranscreator  = C3DTransformCreatorHandler::instance().produce(transform_decr.str());
	
	auto transform = vftranscreator->create(vf->get_size()); 
	CDoubleVector buffer(transform->degrees_of_freedom(), false);
	
	auto ib = buffer.begin(); 
	for (auto ivf = vf->begin(); ivf != vf->end(); ++ivf) {
		cvdebug() << *ivf << "\n"; 
		*ib++ = ivf->x; 
		*ib++ = ivf->y; 
		*ib++ = ivf->z; 
	}
	
	transform->set_parameters(buffer); 
	
	if (!C3DTransformationIOPluginHandler::instance().save(out_filename, *transform)) 
		throw create_exception<runtime_error>( "Unable to save transformation to '", out_filename, "'"); 
	
	return EXIT_SUCCESS;	
}

MIA_MAIN(do_main);
