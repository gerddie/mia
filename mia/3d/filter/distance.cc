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


#include <mia/3d/filter/distance.hh>
#include <mia/core/distance.hh>

#include <mia/core/threadedmsg.hh>
#include <mia/core/parallel.hh>

namespace distance_3d_filter {

using namespace mia; 

using std::vector; 
using std::string;

template <typename T> 
P3DImage C3DDistanceFilter::operator () ( const T3DImage<T>& image) const
{
	C3DFImage *result = new C3DFImage(image.get_size(), image);

	auto transform_x = [result, &image](const C1DParallelRange& range) {
		vector<float> buffer(image.get_size().x); 
		vector<T> in_buffer(image.get_size().x); 
		for (auto z = range.begin(); z != range.end(); ++z) {
			for (size_t y = 0; y < image.get_size().y; ++y) {
				image.get_data_line_x(y, z, in_buffer);
				distance_transform_prepare(in_buffer.begin(), in_buffer.end(), buffer.begin(),
							   __is_mask_pixel<T>::value); 
				distance_transform_inplace(buffer); 
				result->put_data_line_x(y, z, buffer);
			}
		}
	}; 
	
	auto transform_y = [result](const C1DParallelRange& range) {
		vector<float> buffer(result->get_size().y); 
		for (auto z = range.begin(); z != range.end(); ++z) {
			for (size_t x = 0; x < result->get_size().x; ++x) {
				result->get_data_line_y(x, z, buffer);
				distance_transform_inplace(buffer); 
				result->put_data_line_y(x, z, buffer);
			}
		}
	}; 

	auto transform_z = [result](const C1DParallelRange& range) {
		vector<float> buffer(result->get_size().z);
		for (auto y = range.begin(); y != range.end(); ++y) {
			for (size_t x = 0; x < result->get_size().x; ++x) {
				result->get_data_line_z(x, y, buffer);
				distance_transform_inplace(buffer); 
				transform(buffer.begin(), buffer.end(), buffer.begin(), 
					  [](float x) { return sqrtf(x);}); 
				result->put_data_line_z(x, y, buffer);
			}
		}
	}; 
	
	pfor(C1DParallelRange(0, image.get_size().z, 1), transform_x); 
	pfor(C1DParallelRange(0, image.get_size().z, 1), transform_y); 
	pfor(C1DParallelRange(0, image.get_size().y, 1), transform_z); 
	
	return P3DImage(result); 
}

P3DImage C3DDistanceFilter::do_filter(const mia::C3DImage& image) const
{
        return mia::filter(*this, image); 
}

C3DDistanceImageFilterFactory::C3DDistanceImageFilterFactory():
C3DFilterPlugin("distance")
{
}

C3DFilter *C3DDistanceImageFilterFactory::do_create()const
{
        return new C3DDistanceFilter(); 
}
const string C3DDistanceImageFilterFactory::do_get_descr()const
{
        return "Evaluate the 3D distance transform of an image. "
                "If the image is a binary mask, then result of the distance "
                "transform in each point corresponds to the Euclidian distance to the "
                "mask. If the input image is of a scalar pixel value, then the this "
                "scalar is interpreted as heighfield and the per pixel value adds "
                "to the distance."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDistanceImageFilterFactory();
}

}
