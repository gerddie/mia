/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "Distance Mesh to Mask" 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core/cmdlineparser.hh>
#include <mia/2d/imageio.hh>
#include <mia/core/distance.hh>
#include <mia/3d/distance.hh>
#include <mia/mesh/triangularMesh.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Evaluate the distance between a binary shape given as reference and a mesh."}, 
	{pdi_description, "This program takes a binary masks and a triangular mesh as input and "
		 "evaluates the distance of the mesh vertices to the binary mask"
		 "and stores the result in a new mesh."}, 
	{pdi_example_descr, "Evaluate the distance of the mask given in the images maskXXXX.png to the mesh "
	"given in mesh.v and save the result to mesh-dist.v"}, 
	{pdi_example_code, "-i mesh.v -r maskXXXX.png -o mesh-dist.v"}
}; 

struct FDistAcummulator : public TFilter<void> {

	FDistAcummulator(C3DDistance& distance):
		m_distance(distance), 
		m_z(0)
		{
		}
	template <typename T> 
	void operator ()(const T2DImage<T>& image) {
		C2DFImage buf(image.get_size()); 
		distance_transform_prepare(image.begin(), image.end(), buf.begin()); 
		m_distance.push_slice(m_z, buf); 
		++m_z; 
	}
private: 
	C3DDistance& m_distance; 
	int m_z; 
}; 


int do_main( int argc, char *argv[] )
{


	string src_filename;
	string ref_filename;	
	string out_filename;

	const auto& meshio = CMeshIOPluginHandler::instance(); 
	const auto& imageio = C2DImageIOPluginHandler::instance(); 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-mesh", 'i', "input mesh", CCmdOption::required, &meshio));
	options.add(make_opt( ref_filename, "ref-mask", 'r', "reference binary mask", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-mesh", 'o', "output mesh", CCmdOption::required, &meshio));
	
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(ref_filename, start_filenum, end_filenum, format_width);
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);

	C3DDistance distance; 
	FDistAcummulator acc(distance); 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		cvmsg() << "Read slice " << i << " out of " << "[" << start_filenum << ", " << end_filenum << ")\r";
		string src_name = create_filename(src_basename.c_str(), i);
		auto in_image = load_image2d(src_name);
		mia::accumulate(acc, *in_image); 
	}
	cvmsg() << "\n";
	auto in_mesh = meshio.load(src_filename); 
	
	auto iv = in_mesh->vertices_begin(); 
	auto ev = in_mesh->vertices_end(); 
	auto is = in_mesh->scale_begin(); 

	auto len = in_mesh->vertices_size(); 
	unsigned int k = 0; 
	while (iv != ev) {
		++k; 
		if (!(k & 0xF))
			cvmsg() << k << " of " << len << "\r";
		*is++ = distance.get_distance_at(*iv++); 
	}
	cvmsg() << "\n";
	
	if (!meshio.save(out_filename, *in_mesh))
		throw create_exception<runtime_error>("Unabel to save mesh to '", out_filename, "'");
	
	return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
