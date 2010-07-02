/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

NS_MIA_END
