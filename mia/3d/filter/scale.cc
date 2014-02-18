/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <limits>
#include <sstream>
#include <mia/core/msgstream.hh>
#include <mia/core/scaler1d.hh>
#include <mia/core/utils.hh>
#include <mia/3d/filter/scale.hh>

#include <mia/core/threadedmsg.hh>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>



NS_BEGIN(scale_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

template <class T>
C3DScale::result_type do_scale(const T3DImage<T>& src, const C3DBounds& target_size, 
			       const C1DScalar& scaler_x, 
			       const C1DScalar& scaler_y, 
			       const C1DScalar& scaler_z) 
{
	T3DImage<T> *result = new T3DImage<T>(target_size, src);
	
	cvdebug() << "do_scale: " << src.get_size() << " -> " << target_size << "\n"; 


// run x-scaling 
	T3DImage<double> tmp(C3DBounds(target_size.x, src.get_size().y, src.get_size().z)); 

	auto filter_x = [&src, &tmp, &scaler_x](const tbb::blocked_range<size_t>& range) {
		vector<double> in_buffer(src.get_size().x); 
		vector<double> out_buffer(tmp.get_size().x);
		for (auto z = range.begin(); z != range.end(); ++z)
			for (size_t y = 0; y < src.get_size().y; ++y) {
				copy(src.begin_at(0,y,z), src.begin_at(0,y+1,z), in_buffer.begin()); 
				scaler_x(in_buffer, out_buffer); 
				copy(out_buffer.begin(), out_buffer.end(), tmp.begin_at(0,y,z));
			}
		
	};
	
	parallel_for(tbb::blocked_range<size_t>(0, src.get_size().z, 1), filter_x); 

	// run y-scaling 
	T3DImage<double> tmp2(C3DBounds(target_size.x, target_size.y, src.get_size().z)); 

	auto filter_y = [&tmp, &tmp2, &scaler_y](const tbb::blocked_range<size_t>& range) {
		vector<double> in_buffer(tmp.get_size().y); 
		vector<double> out_buffer(tmp2.get_size().y);
		for (auto z = range.begin(); z != range.end(); ++z)
			for (size_t x = 0; x < tmp.get_size().x; ++x) {
				tmp.get_data_line_y(x, z, in_buffer);
				scaler_y(in_buffer, out_buffer); 
				tmp2.put_data_line_y(x, z, out_buffer);
			}
		
	};
	parallel_for(tbb::blocked_range<size_t>(0, tmp.get_size().z, 1), filter_y); 

	auto filter_z = [&tmp2, result, &scaler_z](const tbb::blocked_range<size_t>& range) {
		vector<double> in_buffer(tmp2.get_size().z); 
		vector<double> out_buffer(result->get_size().z);
		vector<T> out_buffer_T(result->get_size().z);
		for (auto y = range.begin(); y != range.end(); ++y)
			for (size_t x = 0; x < tmp2.get_size().x; ++x) {
				tmp2.get_data_line_z(x, y, in_buffer);
				scaler_z(in_buffer, out_buffer); 
				transform(out_buffer.begin(), out_buffer.end(), out_buffer_T.begin(), 
					  [](double x){ return mia_round_clamped<T>(x);}); 
				result->put_data_line_z(x, y, out_buffer_T);
			}
		
	};

	parallel_for(tbb::blocked_range<size_t>(0, tmp2.get_size().y, 1), filter_z); 

	return C3DScale::result_type(result);	
}
			       

template <class T>
C3DScale::result_type C3DScale::operator () (const T3DImage<T>& src) const
{
	TRACE_FUNCTION; 

	C3DBounds target_size( m_size.x ? m_size.x : src.get_size().x, 
 			       m_size.y ? m_size.y : src.get_size().y, 
			       m_size.z ? m_size.z : src.get_size().z ); 

	cvdebug() << "sizes: " << src.get_size() << " -> " << target_size << "\n"; 

	if (target_size == src.get_size())
		return C3DScale::result_type(new T3DImage<T>(src));
	
	
	
	C1DScalar scaler_x(*m_kernel, src.get_size().x, static_cast<size_t>(target_size.x));
	C1DScalar scaler_y(*m_kernel, src.get_size().y, static_cast<size_t>(target_size.y));
	C1DScalar scaler_z(*m_kernel, src.get_size().z, static_cast<size_t>(target_size.z));

	C3DFVector factor(float(src.get_size().x / float(target_size.x)), 
			  float(src.get_size().y / float(target_size.y)), 
			  float(src.get_size().z / float(target_size.z))); 

	cvdebug() << "factor: " << factor << "\n"; 	

	C3DScale::result_type result = do_scale(src, target_size, 
						scaler_x, scaler_y, scaler_z); 
	result->set_voxel_size(src.get_voxel_size() * factor);
	return C3DScale::result_type(result);
}


C3DScale::C3DScale(const C3DBounds& size, PSplineKernel kernel):
	m_size(size),
	m_kernel(kernel)
{
	TRACE_FUNCTION; 
}

C3DScale::result_type C3DScale::do_filter(const C3DImage& image) const
{
	TRACE_FUNCTION; 
	return mia::filter(*this, image);
}


C3DScaleFilterPlugin::C3DScaleFilterPlugin():
	C3DFilterPlugin("scale"),
	m_s(0,0,0)
{
	add_parameter("s", new C3DBoundsParameter(m_s, 0,"target size to set all components at once (component 0:use input image size)"));

	add_parameter("sx", new CUIntParameter(m_s.x, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in x direction (0:use input image size)"));

	add_parameter("sy", new CUIntParameter(m_s.y, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in y direction (0:use input image size)"));

	add_parameter("sz", new CUIntParameter(m_s.z, 0,
					      numeric_limits<unsigned int>::max(), false,
					      "target size in y direction (0:use input image size)"));

	add_parameter("interp", make_param(m_kernel, "bspline:d=3",  false, "interpolation kernel to be used "));
}


C3DFilter *C3DScaleFilterPlugin::do_create()const
{
	return new C3DScale(m_s, m_kernel);
}

const string C3DScaleFilterPlugin::do_get_descr()const
{
	return "3D image filter that scales to a given target size ";
}

C3DScaleFactor::C3DScaleFactor(const C3DFVector& factor, PSplineKernel kernel):
	m_factor(factor), 
	m_kernel(kernel)
{
	TRACE_FUNCTION; 
}

	

template <typename  T>
mia::C3DFilter::result_type C3DScaleFactor::operator () (const mia::T3DImage<T>& src) const
{
	TRACE_FUNCTION; 
	C1DScalar scaler_x(*m_kernel, src.get_size().x, m_factor.x);
	C1DScalar scaler_y(*m_kernel, src.get_size().y, m_factor.y);
	C1DScalar scaler_z(*m_kernel, src.get_size().z, m_factor.z);

	C3DBounds target_size( scaler_x.get_output_size(), 
			       scaler_y.get_output_size(), 
			       scaler_z.get_output_size()); 
	
	if (target_size == src.get_size())
		return C3DScale::result_type(new T3DImage<T>(src));
	
	C3DScale::result_type result = do_scale(src, target_size, 
						scaler_x, scaler_y, scaler_z); 
	result->set_voxel_size(src.get_voxel_size() / m_factor);
	return C3DScale::result_type(result);
}


C3DScale::result_type C3DScaleFactor::do_filter(const mia::C3DImage& image) const
{
	return mia::filter(*this, image); 
}

C3DScaleFactorFilterPlugin::C3DScaleFactorFilterPlugin():
	C3DFilterPlugin("fscale"),
	m_factor(1,1,1)
{
	add_parameter("fx", new CFloatParameter(m_factor.x, 0.0001,
						numeric_limits<unsigned int>::max(), false,
						"scaling factor in x direction"));

	add_parameter("fy", new CFloatParameter(m_factor.y, 0.0001, 10000, false,
					       "scaling factor in y direction"));

	add_parameter("fz", new CFloatParameter(m_factor.z, 0.0001,
						numeric_limits<unsigned int>::max(), false,
						"scaling factor in z direction"));

	add_parameter("interp", make_param(m_kernel, "bspline:d=3",  false, "interpolation kernel to be used "));
	
}
	
mia::C3DFilter *C3DScaleFactorFilterPlugin::do_create()const
{
	return new C3DScaleFactor(m_factor, m_kernel); 
}

const std::string C3DScaleFactorFilterPlugin::do_get_descr()const
{
	return "3D image filter that scales based on the given scale factor";
}




CIsoVoxel::CIsoVoxel(float voxelsize, PSplineKernel kernel):
	m_voxelsize(voxelsize), 
	m_kernel(kernel)
{
}
	
CIsoVoxel::result_type CIsoVoxel::do_filter(const mia::C3DImage& image) const
{
	TRACE_FUNCTION; 
	C3DFVector vs = image.get_voxel_size();
	C3DScaleFactor scaler(vs, m_kernel);
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
	m_voxelsize(1.0)
{
	add_parameter("size", new CFloatParameter(m_voxelsize, 0.001, 1e+6, false,"isometric target voxel size"));
	add_parameter("interp", make_param(m_kernel, "bspline:d=3",  false, "interpolation kernel to be used "));
}

mia::C3DFilter *CIsoVoxelFilterPlugin::do_create()const
{
	return new CIsoVoxel(m_voxelsize, m_kernel); 
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
