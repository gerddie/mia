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

#include <limits>
#include <sstream>

#include <mia/2d/filter.hh>
#include <mia/2d/filter/thresh.hh>

NS_BEGIN(thresh_2dimage_filter)

NS_MIA_USE;
using namespace std; 

C2DThreshNImageFilter::C2DThreshNImageFilter(mia::P2DShape shape, double thresh):
	m_shape(shape), 
	m_thresh(thresh)
{
	
}

template <typename T>
typename C2DFilter::result_type C2DThreshNImageFilter::operator () (const mia::T2DImage<T>& image)const
{
	T2DImage<T> *presult  = new T2DImage<T>(image); 
	P2DImage result(presult); 
	auto  ir = presult->begin(); 
	auto  ii = image.begin(); 
	
	for(size_t y = 0; y < image.get_size().y; ++y) 
		for(size_t x = 0; x < image.get_size().x; ++x, ++ir, ++ii) {
			bool clear = true; 
			for (auto sb = m_shape->begin(); clear && sb !=  m_shape->end(); ++sb) {
				C2DBounds loc(x + sb->x, y + sb->y);
				if (loc < image.get_size()) 
					clear = image(loc) < m_thresh; 
			}
			if (clear) 
				*ir =  T();
		}
	return result; 
}



P2DImage C2DThreshNImageFilter::do_filter(const C2DImage& src) const
{
	return mia::filter(*this, src); 
}

C2DThreshNImageFilterFactory::C2DThreshNImageFilterFactory():
	C2DFilterPlugin("thresh"),
	m_thresh(5.0)
{
	add_parameter("shape", make_param(m_shape, "4n", false, "neighborhood shape to take into account")); 
	add_parameter("thresh", make_param(m_thresh, false, "The threshold value"));
}

C2DFilter *C2DThreshNImageFilterFactory::do_create()const
{
	return new C2DThreshNImageFilter(m_shape, m_thresh);
}

const std::string C2DThreshNImageFilterFactory::do_get_descr() const
{
	return  "This filter sets all pixels of an image to zero that fall below a certain threshold and "
		"whose neighbours in a given neighborhood shape also fall below a this threshold"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DThreshNImageFilterFactory(); 
}

NS_END
