/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_2d_distances_hh
#define mia_2d_distances_hh

#include <mia/2d/2DVector.hh>
#include <mia/2d/defines2d.hh>

NS_MIA_BEGIN

/**
   Evaluate the distance between a point and a löine segment 
   @param point the point 
   @param a start of line segment 
   @param b end of line segment 
   @returns distance 
*/

float EXPORT_2D distance_point_line(const C2DFVector& point,
				    const C2DFVector& a,
				    const C2DFVector& b);



NS_MIA_END

#endif
