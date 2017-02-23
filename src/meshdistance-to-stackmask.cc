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

#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core/cmdlineparser.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/interpolator.hh>
#include <mia/core/distance.hh>
#include <mia/3d/distance.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/mesh/filter.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Evaluate the distance between a binary shape given as reference and a mesh."}, 
	{pdi_description, "This program takes a binary masks and a triangular mesh as input and "
	 "evaluates the distance of the mesh vertices to the binary mask"
	 "and stores the result in a new mesh."
	}, 
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
		distance_transform_prepare(image.begin(), image.end(), buf.begin(), __is_mask_pixel<T>::value); 
		m_distance.push_slice(m_z, buf); 
		++m_z; 
	}
private: 
	C3DDistance& m_distance; 
	int m_z; 
}; 


C2DFImage create_distance_image(const C3DDistance& distance, int slice) 
{
	C2DFImage result = distance.get_distance_slice(slice); 
	transform(result.begin(), result.end(), result.begin(), [](float x){return sqrt(x);}); 
	return result; 
}


int do_main( int argc, char *argv[] )
{
	string src_filename;
	string ref_filename;	
	string out_filename;

	const auto& meshio = CMeshIOPluginHandler::instance(); 
	const auto& imageio = C2DImageIOPluginHandler::instance(); 

	CCmdOptionList options(g_description);
	options.add(make_opt( src_filename, "in-mesh", 'i', "input mesh", CCmdOptionFlags::required_input, &meshio));
	options.add(make_opt( ref_filename, "ref-mask", 'r', "reference binary mask", CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-mesh", 'o', "output mesh", CCmdOptionFlags::required_output, &meshio));
	
	
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

	in_mesh = mia::run_filter(*in_mesh, "vtxsort:dir=[<0,0,1>]"); 
	
	
	auto iv = in_mesh->vertices_begin(); 
	auto ev = in_mesh->vertices_end(); 
	auto is = in_mesh->scale_begin(); 

	int old_z_start = static_cast<int>(floor(iv->z)); 
	int old_z_end   = old_z_start + 1; 

	C2DInterpolatorFactory  ipf("bspline:d=1", "zero"); 

	C2DFImage z_low = create_distance_image(distance, old_z_start); 
	C2DFImage z_high =  create_distance_image(distance, old_z_end); 

	shared_ptr<T2DInterpolator<float> > ipzlow(ipf.create(z_low.data())); 
	shared_ptr<T2DInterpolator<float> > ipzhigh(ipf.create(z_high.data())); 
	
	int n = in_mesh->vertices_size();
	int k = 0; 

	float max_distance=0.0f; 
	float min_distance=numeric_limits<float>::max(); 



	while (iv != ev) {

		if ( (k & 0xFF) == 0) 
			cvmsg() << "processing vertices " <<  k << " of " << n <<  " @ z= " << old_z_start << "\r"; 
		
		// update the input slices if necessary 
		if (iv->z > old_z_end || iv->z < old_z_start ) {
			int z_start = static_cast<int>(floor(iv->z)); 
			int z_end   = z_start + 1;

			cvinfo() << "Slice extrems:[" << setw(5) << old_z_start << ", " << setw(5)<< old_z_end << "]: " 
				 << setw(10)<< min_distance << ", " 
				 << setw(10)<< max_distance << "\n"; 


			max_distance=0.0f;
			min_distance=numeric_limits<float>::max(); 
			
			if (z_start == old_z_end) {
				ipzlow = ipzhigh; 
				ipzhigh.reset(ipf.create(create_distance_image(distance, z_end).data())); 
			} else if (z_end == old_z_start) {
				ipzhigh = ipzlow; 
				ipzlow.reset(ipf.create(create_distance_image(distance, z_start).data())); 
			} else {
				ipzhigh.reset(ipf.create(create_distance_image(distance, z_end).data())); 
				ipzlow.reset(ipf.create(create_distance_image(distance, z_start).data())); 
			}
			old_z_start = z_start; 
			old_z_end = z_end; 
		}
		
		float dz = iv->z - old_z_start; 
		float fz = 1.0 - dz;
		C2DFVector point(iv->x, iv->y); 

		float f_low = (*ipzlow)(point); 
		float f_high = (*ipzhigh)(point);


		*is = f_low * fz + f_high * dz; 

		if (max_distance < *is) {
			max_distance = *is; 
		}

		if (min_distance > *is) {
			min_distance = *is; 
		}
		
		++is; 
		++iv; 
		++k; 
	}
	cvmsg() << "Done                                    \n"; 

	
	if (!meshio.save(out_filename, *in_mesh))
		throw create_exception<runtime_error>("Unabel to save mesh to '", out_filename, "'");
	
	return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
