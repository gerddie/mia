/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/msgstream.hh>
#include <mia/core/utils.hh>
#include <mia/2d/filter/shaped_mean.hh>
#include <boost/type_traits/is_floating_point.hpp>

NS_BEGIN(shapedmean_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DShapedMean::C2DShapedMean(P2DShape shape):
	m_shape(shape)
{
}

template <typename T> 
C2DShapedMean::result_type C2DShapedMean::operator () (const T2DImage<T>& data) const
{
	TRACE_FUNCTION; 

	C2DFImage help(data.get_size()); 
	C2DFImage n(data.get_size()); 

	for (auto is = m_shape->begin(); is != m_shape->end(); ++is) {
		int startinx = 0; 
		int startiny = 0; 
		int startoutx = 0; 
		int startouty = 0; 
		int lenx = data.get_size().x; 
		int endiny = data.get_size().y; 

		if (is->x > 0) {
			startinx = is->x; 
			lenx  -= is->x;
		} else {
			startoutx = -is->x; 
			lenx  += is->x;
		}

		if (is->y > 0) {
			startiny = is->y; 
		} else {
			startouty = -is->y;
			endiny += is->y;
		}
		int oy = startouty; 

		for (int y = startiny; y < endiny; ++y, ++oy) {
			transform(data.begin_at(startinx, y), data.begin_at(startinx, y) + lenx,
				  help.begin_at(startoutx, oy), 
				  help.begin_at(startoutx, oy), 
				  [](float x1, float x2){return x1 + x2;}); 
			
			transform(n.begin_at(startoutx, oy), 
				  n.begin_at(startoutx, oy) + lenx, 
				  n.begin_at(startoutx, oy), [](float x){return x + 1.0f;});  
		}
	}
	
	T2DImage<T> *result = new T2DImage<T>(data.get_size(), data); 
	
	transform(help.begin(), help.end(), n.begin(), result->begin(), 
		  [](float x, float y) { return mia_round_clamped<T>(x/y); });
	return P2DImage(result); 
}

P2DImage C2DShapedMean::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DShapedMeanFilterPlugin::C2DShapedMeanFilterPlugin():
	C2DFilterPlugin("shmean")
{
	add_parameter("shape", make_param(m_shape, "8n", false, "neighborhood shape to evaluate the mean"));
}

C2DFilter *C2DShapedMeanFilterPlugin::do_create()const
{
	return new C2DShapedMean(m_shape);
}

const string C2DShapedMeanFilterPlugin::do_get_descr()const
{
	return "2D image filter that evaluates the mean over a given neighborhood shape";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DShapedMeanFilterPlugin();
}

NS_END

