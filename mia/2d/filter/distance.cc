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

#include <mia/2d/filter/distance.hh>
#include <mia/core/distance.hh>
#include <mia/core/parallel.hh>


NS_BEGIN(distance_2d_filter)

NS_MIA_USE;
using namespace std;


template <typename T>
typename C2DDistanceFilter::result_type
C2DDistanceFilter::operator () (const T2DImage<T>& image) const
{
       auto result = new C2DFImage(image.get_size(), image);
       auto transform_x = [result, &image](const C1DParallelRange & range) {
              vector<float> buffer(image.get_size().x);
              vector<T> in_buffer(image.get_size().x);

	      for (auto y = range.begin(); y < range.end(); ++y) {
		      image.get_data_line_x(y, in_buffer);
		      distance_transform_prepare(in_buffer.begin(), in_buffer.end(), buffer.begin(),
						 __is_mask_pixel<T>::value);
		      distance_transform_inplace(buffer);
		      result->put_data_line_x(y, buffer);
	      }
       };
       auto transform_y = [result](const C1DParallelRange & range) {
              vector<float> buffer(result->get_size().y);

	      for (auto x = range.begin(); x < range.end(); ++x) {
		      result->get_data_line_y(x, buffer);
		      distance_transform_inplace(buffer);
		      transform(buffer.begin(), buffer.end(), buffer.begin(),
				[](float v) {
					return sqrtf(v);
				});

		      result->put_data_line_y(x, buffer);
	      }
       };
       pfor(C1DParallelRange(0, image.get_size().y, 1), transform_x);
       pfor(C1DParallelRange(0, image.get_size().x, 1), transform_y);

       return P2DImage(result);
}

P2DImage C2DDistanceFilter::do_filter(const C2DImage& image) const
{
       return ::mia::filter(*this, image);
}

C2DDistanceImageFilterFactory::C2DDistanceImageFilterFactory():
       C2DFilterPlugin("distance")
{
}

C2DFilter *C2DDistanceImageFilterFactory::do_create() const
{
       return new C2DDistanceFilter();
}

const string C2DDistanceImageFilterFactory::do_get_descr()const
{
       return "2D image distance filter, evaluates the distance map for a binary mask.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DDistanceImageFilterFactory();
}

NS_END
