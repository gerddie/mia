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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <mia/mesh/triangularMesh.hh>
#include <mia/3d/image.hh>
#include <mia/3d/imageio.hh>

#include <mia/core/cmdlineparser.hh>
#include <mia/internal/main.hh>
#include <gts.h>

using namespace std; 
using namespace mia;


GtsSurface *iso_surface(const C3DImage& src, gfloat iso_value, gint max_edges, gint max_faces, 
				gdouble max_cost, gboolean bordered, 
				gfloat coarsen_method_factor);


CTriangleMesh *gts_to_mona_mesh(GtsSurface *surface, bool reverse_winding);

const SProgramDescription g_description = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Extract an ist-surface froma a 3D image."}, 
	{pdi_description, "This program is used to extract an iso-surface from the input gray scale "
	 "image by using marching thetrahedra."},
	{pdi_example_descr, "Extract the surface corresponding to the value 30 and stop optimizing "
		 "when the mesh consists of less than 100000 triangles."}, 
	{pdi_example_code, "-i image.v -o mesh.vmesh -s 30 -f 100000"  }
}; 


int do_main (int argc, char * argv[])
{
	
	string in_filename; 
	string out_filename; 

	float iso_value = 128.0f; 
	bool use_border = false; 
	bool reverse_winding = false; 
	
	gint max_faces = -1; 
	gint max_edges = -1; 
	gfloat  max_cost = 0.1; 
	gfloat factor = 4.0; 
	const auto& imageio = C3DImageIOPluginHandler::instance();
	const auto& meshio = CMeshIOPluginHandler::instance(); 

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt(  in_filename, "in-file", 'i', "input image", CCmdOptionFlags::required_input, &imageio )); 
	options.add(make_opt(  out_filename, "out-file", 'o', "output mesh", CCmdOptionFlags::required_output, &meshio )); 

	options.set_group("Image options"); 
	options.add(make_opt(  iso_value, "iso-value", 's', "iso-value of iso surface to be extracted")); 
	options.add(make_opt(  use_border, "bordered", 'b', "put an empty border around the image to ensure a closed surface")); 

	
	options.set_group("Mesh options"); 
	options.add(make_opt(  max_faces, "max-faces", 'f', "maximum number of Faces,")); 
	options.add(make_opt(  max_edges, "max-edges", 'e', "maximum number of Edges")); 
	options.add(make_opt(  max_cost, "max-cost", 'c', "maximum cost for edge collaps")); 
	options.add(make_opt(  factor, "ratio", 'r', "ratio of faces(edges) to target number, at which  "
			       "the optimization changes from edge-length to volume-optimized"));
	options.add(make_opt(  reverse_winding, "reverse-winding", 'w', "reverse the winding of the triangles in order "
			       "to make normals point away from the high intensity area")); 

	
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 
	
	
        auto image = load_image<P3DImage>(in_filename); 
	if (!image) 
		throw create_exception<invalid_argument>("No image data found in '", in_filename, "'"); 
	
	GtsSurface *surface = iso_surface(*image, iso_value, max_edges, max_faces, 
					  max_cost, use_border, factor);

	if (surface) {
		unique_ptr<CTriangleMesh> mesh(gts_to_mona_mesh(surface, reverse_winding));
		gts_object_destroy((GtsObject*)surface);
		
		if ( !CMeshIOPluginHandler::instance().save(out_filename, *mesh) ) {
			throw runtime_error("Unable to write mesh to " + out_filename);
		}
	}else {
		throw runtime_error("Unable to creat iso-surface."); 
	}
	return EXIT_SUCCESS; 
}

MIA_MAIN(do_main); 
