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
#include <mia/internal/main.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/imagedraw.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Create a binary mask from the mesh surfac."}, 
	{pdi_description, "This program converts a 3D mesh into a binary mask comprising the mesh surface. "
         "The output image size is either given on the command line or deducted from the mesh dimensions."}, 
	{pdi_example_descr, "The input mesh input.vmesh is converted into the binary mask mask.v. "
         "The image size is set to 50,50,50 and the size is obtained automatically to include the whole mesh"}, 
	{pdi_example_code, "-i input.vmesh -o mask.v --origin 0,0,0"}
}; 





int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
        C3DFVector origin = C3DFVector::_0; 
        C3DFVector spacing = C3DFVector::_1;
        C3DBounds size = C3DBounds::_0; 

	const auto& meshio = CMeshIOPluginHandler::instance(); 
	const auto& imgio = C3DImageIOPluginHandler::instance(); 


        CCmdOptionList options(g_general_help);
        options.set_group("File I/O"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input mesh to be adapted", 
                              CCmdOptionFlags::required_input, &meshio));
	options.add(make_opt( out_filename, "out-file", 'o', "output image containing the binary mask", 
                              CCmdOptionFlags::required_output, &imgio));

        options.set_group("Properties"); 
        
        options.add(make_opt(origin, "origin", 0, "coordinate of pixel at index 0,0,0"));
        options.add(make_opt(spacing, "spacing", 0, "voxel spacing in the output image, all values must be positive."));
        options.add(make_opt(size, "size", 0, "size of the output image (0,0,0 = deduct)"));
        
                
        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
        
        auto mesh = meshio.load(in_filename); 
	if (!mesh) 
		throw create_exception<invalid_argument>( "No mesh found in '", in_filename, "'"); 

        if (spacing.x <= 0.0f || spacing.y <= 0.0f || spacing.z <= 0.0f) 
                throw create_exception<invalid_argument>("Voxel spacing must  be positive, but you gave ", spacing); 

	


        if (size == C3DBounds::_0) {
                // evaluate required output size i.e. find larest vertex coordinates and their 
                // distance from the origin, and multipyl this with the spacing. 
                C3DFVector d = origin; 
                for_each(mesh->vertices_begin(), mesh->vertices_end(), [&origin, &d](const C3DFVector v) {
                                if (d.x < v.x) d.x = v.x; 
                                if (d.z < v.z) d.z = v.z; 
                                if (d.y < v.y) d.y = v.y; 
                        }); 
                d *= spacing; 
                d.x = ceil(d.x); 
                d.y = ceil(d.y); 
                d.z = ceil(d.z); 
                
                size = C3DBounds(d) + C3DBounds::_1;
        }
        
        C3DBitImageDrawTarget output(size, origin, spacing);
	
	typedef CTriangleMesh::triangle_type CTriangle; 

	for_each(mesh->triangles_begin(), mesh->triangles_end(), 
		 [&output, &mesh](const CTriangle& t) {
			 output.draw_triangle(mesh->vertex_at(t.x), 
					      mesh->vertex_at(t.y), 
					      mesh->vertex_at(t.z));});
	
	auto image = output.get_image(); 
	save_image(out_filename, image); 
        
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 

