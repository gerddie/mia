/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
	typedef typename T3DImage<T>::const_range_iterator_with_boundary_flag range_iterator_with_boundary; 
	typedef typename range_iterator_with_boundary::EBoundary  EBoundary; 
	int dx = data.get_size().x; 
	int dxy = dx * data.get_size().y; 

	auto iv = vf.begin(); 
	auto e = data.end_range(C3DBounds::_0, data.get_size()).with_boundary_flag(); 
	float maxnorm = 0.0; 
	for (auto d = data.begin_range(C3DBounds::_0, data.get_size()).with_boundary_flag(); 
	     d != e; ++iv, ++d) {
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
