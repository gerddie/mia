/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/* 
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Norm of a gradient of the image}
   \label{filter3d:gradnorm}
   
   \begin{description}
   
   \item [Plugin:] gradnorm
   \item [Description:] This filter evaluates the norm of the gradient of the input image
   \item [Input:] An abitrary gray scale image. 
   \item [Input:] The norm of the gradient of the input image as a single float valued 
             image normalized to the range [-1, 1]. 
   
   \end{description}
   This plugin doesn't take parameters. 

   LatexEnd  
 */


#include <limits>
#include <algorithm>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/gradnorm.hh>



NS_BEGIN(gradnorm_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;

CGradnorm::CGradnorm()
{
}

template <class T>
CGradnorm::result_type CGradnorm::operator () (const T3DImage<T>& data) const
{
	TRACE("CGradnorm::operator ()");

	C3DFVectorfield vf(data.get_size()); 
	typedef typename T3DImage<T>::range_iterator::EBoundary  EBoundary; 
	int dx = data.get_size().x; 
	int dxy = dx * data.get_size().y; 

	auto iv = vf.begin(); 
	auto e = data.end_range(C3DBounds::_0, data.get_size()); 
	float maxnorm = 0.0; 
	for (auto d = data.begin_range(C3DBounds::_0, data.get_size()); d != e; ++iv, ++d) {
		auto i = d.get_point(); 
		iv->x = d.get_boundary_flags() & EBoundary::eb_x ? 0.0 : i[1] - i[-1]; 
		iv->y = d.get_boundary_flags() & EBoundary::eb_y ? 0.0 : i[dx] - i[-dx]; 
		iv->z = d.get_boundary_flags() & EBoundary::eb_z ? 0.0 : i[dxy] - i[-dxy];
		auto n = iv->norm2(); 
		if (maxnorm < n) 
			maxnorm = n; 
	}
	
	T3DImage<float> *result = new T3DImage<float>(data.get_size(), data);
	if (maxnorm > 0.0) {
		maxnorm = sqrt(maxnorm); 
		transform(vf.begin(), vf.end(), result->begin(), 
			  [&maxnorm](const C3DFVector& v){return v.norm()/maxnorm;}); 
	}
	return CGradnorm::result_type(result);
}

CGradnorm::result_type CGradnorm::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}


C3DGradnormFilterPlugin::C3DGradnormFilterPlugin():
	C3DFilterPlugin("gradnorm")
{
}

C3DFilter *C3DGradnormFilterPlugin::do_create()const
{
	return new CGradnorm();
}

const string C3DGradnormFilterPlugin::do_get_descr()const
{
	return "3D image to gradient norm filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DGradnormFilterPlugin();
}

NS_END
