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
#include <mia/mesh/triangularMesh.hh>
#include <mia/3d/imageio.hh>

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



void fill_flat_bottom_triangle(C3DBitImage& target, const C3DFVector& c1, 
                               const C3DFVector& c2, const C3DFVector& c3)
{
        float invslope1 = (c2.x - c1.x) / (c2.y - c1.y);
        float invslope2 = (c3.x - c1.x) / (c3.y - c1.y);
        
        float curx1 = v1.x;
        float curx2 = v1.x;
        
        for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
                {
                        drawLine((int)curx1, scanlineY, (int)curx2, scanlineY);
                        curx1 += invslope1;
                        curx2 += invslope2;
  }
}


void fill_flat_top_triangle(C3DBitImage& target, const C3DFVector& c1, 
                            const C3DFVector& c2, const C3DFVector& c3)
{
        float invslope1 = (c2.x - c1.x) / (c2.y - c1.y);
        float invslope2 = (c3.x - c1.x) / (c3.y - c1.y);
        
        float curx1 = c1.x;
        float curx2 = c2.x;
        
        for (int scanlineY = c1.y; scanlineY <= c2.y; scanlineY++) {
                drawLine(target, (int)curx1, scanlineY, (int)curx2, scanlineY, c1.z);
                curx1 += invslope1;
                curx2 += invslope2;
        }
}


void fill_triangle_y(C3DBitImage& target, const CTriangleMesh::triangle_type& triangle)
{
        C3DFVector c1 = triangle.x; 
        C3DFVector c2 = triangle.y; 
        C3DFVector c3 = triangle.z; 

        if (c1.y < c3.y) swap(c1,c3); 
        if (c1.y < c2.y) swap(c1,c2);
        if (c2.y < c3.y) swap(c2,c3);

        if (c2.y == c1.y)  {
                
                fill_flat_bottom_triangle_y(target, c1, c2, c3);
        
        } else if (c2.y == c3.y) {
                
                fill_flat_top_triangle_y(target, c1, c2, c3);
        } else {
                C3DVector c4(c1.x + ((c2.y - c1.y) / (c3.y - c1.y)) * (c3.x - c1.x), c2.y, c1.z);
                
                fillBottomFlatTriangle(target, c1, c2, c4);
                fillTopFlatTriangle(target, c2, c4, c3);
        }

}

void fill_triangle(C3DBitImage& target, const CTriangleMesh::triangle_type& triangle) 
{
        if (triangle.x.z == triangle.y.z && triangle.y.z == triangle.z.z)
                fill_triangle_xy(target, triangle); 

        C3DFVector c1 = triangle.x; 
        C3DFVector c2 = triangle.y; 
        C3DFVector c3 = triangle.z; 

        if (c1.z < c3.z) swap(c1,c3); 
        if (c1.z < c2.z) swap(c1,c2);
        if (c2.z < c3.z) swap(c2,c3);
        

                

        


        

}


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
                for_each(mesh->vertices_begin(), mesh->vertices_end(), [&origin](const C3DFVector v) {
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
        
        C3DBitImage output(size); 
        
        

        
        
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 

