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

#include <cassert>
#include <mia/core/msgstream.hh>
#include <mia/2d/distances.hh>

NS_MIA_BEGIN

float EXPORT_2D distance_point_line(const C2DFVector& point,
				    const C2DFVector& a,
				    const C2DFVector& b)
{
	if (point == a || point == b)
		return 0.0;

	const C2DFVector line = a - b;
	const C2DFVector p_end = point - b;
	const float nline =  line.norm2();
	const float npoint = p_end.norm2();
	if (nline > 0.0 ) {
		const float dotlplt = dot(line, p_end) / nline;

		// point is "behind" reference end point or other end point
		if (dotlplt <= 0)
			return sqrt(npoint);
		else if (dotlplt >= 1.0)
			return (point - a).norm();

		const C2DFVector proj = b + dotlplt * line;
		return (point - proj).norm();
	}else // special case: line segment is actually a point
		return sqrt(npoint);
}

struct FDistanceTransform : public TFilter<C2DFImage> {
	template <typename T> 
	C2DFImage operator()( const T2DImage<T>& image) const {
		
		C2DFImage result(image.get_size(), image); 
		vector<float> buffer(image.get_size().x); 
		for (size_t y = 0; y < image.get_size().y; ++y) {
			image.get_data_line_x(y, buffer);
			dt1d(buffer); 
			image.put_data_line_x(y, buffer);
		}
		buffer.resize(image.get_size().y); 
		for (size_t x = 0; x < image.get_size().x; ++x) {
			image.get_data_line_y(x, buffer);
			dt1d(buffer); 
			image.put_data_line_y(x, buffer);
		}
		
	}; 

}; 


C2DFImage EXPORT_2D distance_transform(const C2DImage& f)
{

	TRACE_FUNCTION; 
	vector<float> buffer(f.get_size().x); 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		image.get_data_line_x(y, buffer);
		dt1d(buffer); 
		image.put_data_line_x(y, buffer);
	}
	buffer.resize(image.get_size().y); 
	for (size_t x = 0; x < image.get_size().x; ++x) {
		image.get_data_line_y(x, buffer);
		dt1d(buffer); 
		image.put_data_line_y(x, buffer);
	}

}

NS_MIA_END
