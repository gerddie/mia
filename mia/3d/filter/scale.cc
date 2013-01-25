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

#include <limits>
#include <sstream>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/scaler1d.hh>
#include <mia/core/utils.hh>
#include <mia/3d/filter/scale.hh>



NS_BEGIN(scale_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

C3DScale::C3DScale(const C3DBounds& size, const string& filter):
	m_size(size),
	m_ipf(new C3DInterpolatorFactory(produce_spline_kernel(filter), "mirror"))
{
	TRACE_FUNCTION; 
}

template <class T>
C3DScale::result_type C3DScale::operator () (const T3DImage<T>& src) const
{
	TRACE_FUNCTION; 
	if (src.get_size() == m_size)
		return C3DScale::result_type(new T3DImage<T>(src));

	C3DBounds target_size( m_size.x ? m_size.x : src.get_size().x, 
 			       m_size.y ? m_size.y : src.get_size().y, 
			       m_size.z ? m_size.z : src.get_size().z ); 
	
	T3DImage<T> *result = new T3DImage<T>(target_size, src);
	
	C1DScalarFixed scaler_x(*m_ipf->get_kernel(), src.get_size().x, target_size.x);
	C1DScalarFixed scaler_y(*m_ipf->get_kernel(), src.get_size().y, target_size.y);
	C1DScalarFixed scaler_z(*m_ipf->get_kernel(), src.get_size().z, target_size.z);

	C3DFVector factor(float(src.get_size().x / float(target_size.x)), 
			  float(src.get_size().y / float(target_size.y)), 
			  float(src.get_size().z / float(target_size.z))); 

	// run x-scaling 
	T3DImage<double> tmp(C3DBounds(target_size.x, src.get_size().y, src.get_size().z)); 
	for (size_t z = 0; z < src.get_size().z; ++z) {
		for (size_t y = 0; y < src.get_size().y; ++y) {
			copy(src.begin_at(0,y,z), src.begin_at(0,y+1,z), scaler_x.input_begin()); 
			scaler_x.run(); 
			copy(scaler_x.output_begin(), scaler_x.output_end(), tmp.begin_at(0,y,z)); 
		}
	}

	// run y-scaling 
	T3DImage<double> tmp2(C3DBounds(target_size.x, target_size.y, src.get_size().z)); 
	vector<double> in_buffer(src.get_size().y); 
	vector<double> out_buffer(target_size.y);
	for (size_t z = 0; z < src.get_size().z; ++z) {
		for (size_t x = 0; x < target_size.x; ++x) {
			tmp.get_data_line_y(x, z, in_buffer);
			copy(in_buffer.begin(), in_buffer.end(), scaler_y.input_begin()); 
			scaler_y.run(); 
			copy(scaler_y.output_begin(), scaler_y.output_end(), out_buffer.begin());
			tmp2.put_data_line_y(x, z, out_buffer);
		}
	}

	// run z-scaling 
	in_buffer.resize(src.get_size().z); 
	vector<T> out_buffer_t(target_size.z);
	for (size_t y = 0; y < target_size.y; ++y) {
		for (size_t x = 0; x < target_size.x; ++x) {
			tmp2.get_data_line_z(x, y, in_buffer);
			copy(in_buffer.begin(), in_buffer.end(), scaler_z.input_begin()); 
			scaler_z.run(); 
			transform(scaler_z.output_begin(), scaler_z.output_end(), out_buffer_t.begin(), 
				  [](double x){ return mia_round<T>(x); }); 
			result->put_data_line_z(x, y, out_buffer_t);
		}
	}

	result->set_voxel_size(src.get_voxel_size() * factor);
	return C3DScale::result_type(result);
}

C3DScale::result_type C3DScale::do_filter(const C3DImage& image) const
{
	TRACE_FUNCTION; 
	return mia::filter(*this, image);
}


C3DScaleFilterPlugin::C3DScaleFilterPlugin():
	C3DFilterPlugin("scale"),
	m_s(0,0,0), 
	m_interp("bspline:d=3")
{
	add_parameter("sx", new CUIntParameter(m_s.x, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in x direction (0:use input image size)"));

	add_parameter("sy", new CUIntParameter(m_s.y, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in y direction (0:use input image size)"));

	add_parameter("sz", new CUIntParameter(m_s.z, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in y direction (0:use input image size)"));

	add_parameter("s", new C3DBoundsParameter(m_s, 0,"target size (component 0:use input image size)"));

	add_parameter("interp", new CStringParameter(m_interp, false, "interpolation method to be used "));
}


C3DFilter *C3DScaleFilterPlugin::do_create()const
{
	return new C3DScale(m_s, m_interp);
}

const string C3DScaleFilterPlugin::do_get_descr()const
{
	return "3D image scale filter";
}


CIsoVoxel::CIsoVoxel(float voxelsize, const std::string& interpolator):
	m_voxelsize(voxelsize), 
	m_interp(interpolator)
{
}
	
CIsoVoxel::result_type CIsoVoxel::do_filter(const mia::C3DImage& image) const
{
	TRACE_FUNCTION; 
	C3DFVector vs = image.get_voxel_size();
	
	C3DBounds target_size(static_cast<unsigned short>(image.get_size().x * fabs(vs.x)/m_voxelsize), 
			      static_cast<unsigned short>(image.get_size().y * fabs(vs.y)/m_voxelsize), 
			      static_cast<unsigned short>(image.get_size().z * fabs(vs.z)/m_voxelsize)); 
	cvdebug() << "scale image to " <<target_size <<  "\n"; 
		   
	// todo: with this approach, the voxel size may not become exactly what is requested. 
	// this would require more handywork. 
	C3DScale scaler(target_size, m_interp);
	return  scaler.filter(image); 
}

CIsoVoxel::result_type CIsoVoxel::do_filter(P3DImage image) const
{
	TRACE_FUNCTION; 
	C3DFVector vs = image->get_voxel_size();
	if (vs.x != m_voxelsize || vs.y != m_voxelsize || vs.z != m_voxelsize) 
		return do_filter(*image); 
	return image;
}



CIsoVoxelFilterPlugin::CIsoVoxelFilterPlugin():
	C3DFilterPlugin("isovoxel"),
	m_voxelsize(1.0),
	m_interp("bspline:d=3")
{
	add_parameter("size", new CFloatParameter(m_voxelsize, 0.001, 1e+6, false,"isometric target voxel size"));
	add_parameter("interp", new CStringParameter(m_interp, false, "interpolation method to be used"));
	
}

mia::C3DFilter *CIsoVoxelFilterPlugin::do_create()const
{
	return new CIsoVoxel(m_voxelsize, m_interp); 
}

const std::string CIsoVoxelFilterPlugin::do_get_descr()const
{
	return "This filter scales an image to make the voxel size isometric "
		"and its size to correspond to the given value"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	auto retval = new C3DScaleFilterPlugin();
	retval->append_interface(new CIsoVoxelFilterPlugin()); 
	return retval; 
}

NS_END
