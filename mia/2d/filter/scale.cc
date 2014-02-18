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
#include <mia/core/spacial_kernel.hh>
#include <mia/core/scaler1d.hh>
#include <mia/core/utils.hh>
#include <mia/2d/filter/scale.hh>


NS_BEGIN(scale_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

CScale::CScale(const C2DBounds& size, PSplineKernel kernel):
	m_size(size),
	m_kernel(kernel)
{

}

CScale::~CScale()
{
}

template <class T>
CScale::result_type CScale::operator () (const T2DImage<T>& src) const
{
	if (src.get_size() == m_size)
		return CScale::result_type(new T2DImage<T>(src));

	// todo: if  the target size equals the input size 
	// one could avoid the copying

	C2DBounds target_size(m_size.x ? m_size.x  : src.get_size().x, 
			      m_size.y ? m_size.y  : src.get_size().y); 


	T2DImage<T> *result = new T2DImage<T>(target_size);
	C2DFVector factor(float(src.get_size().x / float(target_size.x) ),
			  float(src.get_size().y / float(target_size.y) ));
	
	C1DScalar scaler_x(*m_kernel, static_cast<size_t>(src.get_size().x), static_cast<size_t>(target_size.x));
	C1DScalar scaler_y(*m_kernel, static_cast<size_t>(src.get_size().y), static_cast<size_t>(target_size.y));

	// run x-scaling 
	T2DImage<double> tmp(C2DBounds(target_size.x, src.get_size().y)); 
	for (size_t y = 0; y < src.get_size().y; ++y) {
		copy(src.begin_at(0,y), src.begin_at(0,y+1), scaler_x.input_begin()); 
		scaler_x.run(); 
		copy(scaler_x.output_begin(), scaler_x.output_end(), tmp.begin_at(0,y)); 
	}

	// run y-scaling 
	vector<double> in_buffer(src.get_size().y); 
	vector<T> out_buffer(target_size.y);
	for (size_t x = 0; x < tmp.get_size().x; ++x) {
		tmp.get_data_line_y(x, in_buffer);
		copy(in_buffer.begin(), in_buffer.end(), scaler_y.input_begin()); 
		scaler_y.run(); 
		transform(scaler_y.output_begin(), scaler_y.output_end(), out_buffer.begin(), 
			  [](double x){ return mia_round<T>(x); }); 
		result->put_data_line_y(x, out_buffer);
	}

	result->set_pixel_size(src.get_pixel_size() / factor);
	return CScale::result_type(result);
}

CScale::result_type CScale::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DScaleFilterPlugin::C2DScaleFilterPlugin():
	C2DFilterPlugin("scale"),
	m_s(0,0)

{
	add_parameter("sx", new CUIntParameter(m_s.x, 0,
					       numeric_limits<unsigned int>::max(), false,
					       "target size in x direction, 0: use input size"));
	
	add_parameter("sy", new CUIntParameter(m_s.y, 0,
					       numeric_limits<unsigned int>::max(), false,
					       "target size in y direction, 0: use input size"));
	
	add_parameter("s", new C2DBoundsParameter(m_s, false, "target size as 2D vector"));
	add_parameter("interp", make_param(m_interp,"bspline:d=3", false,
					   "interpolation method to be used "));
}


C2DFilter *C2DScaleFilterPlugin::do_create()const
{
	return new CScale(m_s, m_interp);
}

const string C2DScaleFilterPlugin::do_get_descr()const
{
	return "2D image downscale filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DScaleFilterPlugin();
}

NS_END
