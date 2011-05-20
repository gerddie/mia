/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/* 
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Scaling filter}
   \label{filter3d:scale}
   
   \begin{description}
   
   \item [Plugin:] scale
   \item [Description:] A 3D image filter to scale images. 
   \item [Input:] An abitrary gray scale image. 
   \item [Input:] The scaled image with the same pixel type.  
   
   \plugtabstart
   sx & int & target width (0: use input image value) )& 0 \\
   sy & int & target height (0: use input image value) & 0 \\
   sz & int & target height (0: use input image value) & 0 \\
   s & 3D-Vector & target size (component 0: use input image value)  & <0,0,0> \\
   interp & string & interpolation kernel descriptor & "bspline3" \\\hline 
   \plugtabend

   \item [Remark:] Currently downscaling is not handled correctly. 
   
   \end{description}

   LatexEnd  
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

CScale::CScale(const C3DBounds& size, const string& filter):
	m_size(size),
	m_ipf(create_3dinterpolation_factory(GInterpolatorTable.get_value(filter.c_str())))
{

}

CScale::~CScale()
{
}

template <class T>
CScale::result_type CScale::operator () (const T3DImage<T>& src) const
{
	if (src.get_size() == m_size)
		return CScale::result_type(new T3DImage<T>(src));

	C3DBounds target_size( m_size.x ? m_size.x : src.get_size().x, 
			       m_size.y ? m_size.y : src.get_size().y, 
			       m_size.z ? m_size.z : src.get_size().z ); 
	
	T3DImage<T> *result = new T3DImage<T>(target_size);
	
	C1DScalarFixed scaler_x(*m_ipf->get_kernel(), src.get_size().x, target_size.x);
	C1DScalarFixed scaler_y(*m_ipf->get_kernel(), src.get_size().y, target_size.y);
	C1DScalarFixed scaler_z(*m_ipf->get_kernel(), src.get_size().z, target_size.z);

	C3DFVector factor(float(target_size.x) / float(src.get_size().x), 
			  float(target_size.y) / float(src.get_size().y), 
			  float(target_size.z) / float(src.get_size().z)); 

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
			cvdebug() << x << ", " << z << ":" << in_buffer << "\n"; 
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
	return CScale::result_type(result);
}

CScale::result_type CScale::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DScaleFilterPlugin::C3DScaleFilterPlugin():
	C3DFilterPlugin("scale"),
	m_s(0,0,0), 
	m_interp("bspline3")
{
	add_parameter("sx", new CUIntParameter(m_s.x, 0,
					      numeric_limits<unsigned int>::max(), true,
					      "target size in x direction (0:use input image size)"));

	add_parameter("sy", new CUIntParameter(m_s.y, 0,
					      numeric_limits<unsigned int>::max(), true,
					      "target size in y direction (0:use input image size)"));

	add_parameter("sz", new CUIntParameter(m_s.z, 0,
					      numeric_limits<unsigned int>::max(), true,
					      "target size in y direction (0:use input image size)"));

	add_parameter("s", new C3DBoundsParameter(m_s, 0,"target size (component 0:use input image size)"));

	add_parameter("interp", new CStringParameter(m_interp, false, "interpolation method to be used "));
}


C3DScaleFilterPlugin::ProductPtr C3DScaleFilterPlugin::do_create()const
{
	return C3DScaleFilterPlugin::ProductPtr(new CScale(m_s, m_interp));
}

const string C3DScaleFilterPlugin::do_get_descr()const
{
	return "3D image scale filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DScaleFilterPlugin();
}

NS_END