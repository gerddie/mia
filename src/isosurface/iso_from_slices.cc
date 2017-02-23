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


#include <mia/mesh/triangularMesh.hh>
#include <mia/3d/image.hh>
#include <mia/3d/imageio.hh>

#include <mia/core/cmdlineparser.hh>
#include <mia/internal/main.hh>
#include <gts.h>

using namespace std; 
using namespace mia;


GtsSurface *iso_surface(const vector<string>& src, gfloat iso_value, gint max_edges, gint max_faces, 
			gdouble max_cost, gboolean bordered, 
			gfloat coarsen_method_factor);


CTriangleMesh *gts_to_mona_mesh(GtsSurface *surface, bool reverse_winding);

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Extract an ist-surface froma a 3D image."}, 
	{pdi_description, "This program is used to extract an iso-surface from the input gray scale "
	 "image given as slices."},
	{pdi_example_descr, "Extract the surface from an image set with numbers imageXXXX.png that "
	 "corresponds to  the value 30 and stop optimizing when the mesh consists of less than 100000 "
	 "triangles."}, 
	{pdi_example_code, "-i image0000.png -o mesh.vmesh -s 30 -f 100000"  }
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

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt(  in_filename, "in-image", 'i', "input image filename pattern", CCmdOptionFlags::required_input )); 
	options.add(make_opt(  out_filename, "out-mesh", 'o', "output mesh", CCmdOptionFlags::required_output )); 

	options.set_group("Image options"); 
	options.add(make_opt(  iso_value, "iso-value", 's', "iso-value of iso surface to be extracted")); 
	//	options.add(make_opt(  use_border, "bordered", 'b', "put an empty border around the image to ensure a closed surface")); 
	
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
	
	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;
	
	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);
	
	
	vector<string> slices; 
	slices.reserve(end_filenum - start_filenum); 
	for (size_t i = start_filenum; i < end_filenum; ++i)
		slices.push_back(create_filename(src_basename.c_str(), i));
		
	GtsSurface *surface = iso_surface(slices, iso_value, max_edges, max_faces, 
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
