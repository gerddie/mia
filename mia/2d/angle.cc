/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <cmath>
#include <mia/2d/angle.hh>

NS_MIA_BEGIN
float EXPORT_2D angle(const C2DFVector& ray_a, const C2DFVector& ray_b)
{

	const double norm = ray_a.norm() * ray_b.norm(); 
	if (norm == 0.0) 
		return 0.0; 
	
	const double ddot = dot(ray_a, ray_b); 
	return (cross(ray_a, ray_b) >= 0.0 ) ? acos(ddot / norm): 2*M_PI - acos(ddot / norm);
}

float EXPORT_2D angle(const C2DFVector& a, const C2DFVector& b, const C2DFVector& s)
{
	return angle(a - s, b - s); 
}


NS_MIA_END
