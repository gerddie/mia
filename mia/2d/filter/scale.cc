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
   LatexBeginPluginDescription{2D image filters}
   
   \subsection{Scaling filter}
   \label{filter2d:scale}
   
   \begin{description}
   
   \item [Plugin:] scale
   \item [Description:] A 2D image filter to scale images. 
   \item [Input:] An abitrary gray scale image. 
   \item [Input:] The scaled image with the same pixel type.  
   
   \plugtabstart
   sx & int & target width, (0: use input width) & 0 \\
   sy & int & target height (0: use input height) & 0 \\
   s & 2D Vector & target size (component=0: use input height) & <0,0> \\
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
#include <mia/2d/filter/scale.hh>


NS_BEGIN(scale_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

CScale::CScale(const C2DBounds& size, const string& kernel):
	m_size(size),
	m_ipf(new C2DInterpolatorFactory(produce_spline_kernel(kernel), 
					 *produce_spline_boundary_condition("mirror"), 
					 *produce_spline_boundary_condition("mirror")))
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
	typename T2DImage<T>::iterator i = result->begin();
	C2DFVector factor(float(src.get_size().x / float(target_size.x) ),
			  float(src.get_size().y / float(target_size.y) ));
	
	C1DScalarFixed scaler_x(*m_ipf->get_kernel(), src.get_size().x, target_size.x);
	C1DScalarFixed scaler_y(*m_ipf->get_kernel(), src.get_size().y, target_size.y);

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
	m_s(0,0),
	m_interp("bspline:d=3")
{
	add_parameter("sx", new CUIntParameter(m_s.x, 0,
					       numeric_limits<unsigned int>::max(), false,
					       "target size in x direction, 0: use input size"));
	
	add_parameter("sy", new CUIntParameter(m_s.y, 0,
					       numeric_limits<unsigned int>::max(), false,
					       "target size in y direction, 0: use input size"));
	
	add_parameter("s", new C2DBoundsParameter(m_s, false, "target size as 2D vector"));

	
	add_parameter("interp", new CStringParameter(m_interp, false,
						     "interpolation method to be used "));
}


C2DFilter *C2DScaleFilterPlugin::do_create()const
{
	return new CScale(m_s, m_interp);
}

const string C2DScaleFilterPlugin::do_get_descr()const
{
	return "2D image downscaler filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DScaleFilterPlugin();
}

NS_END
