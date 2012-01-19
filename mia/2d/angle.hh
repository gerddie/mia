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

#ifndef mia_2d_angle_hh
#define mia_2d_angle_hh
#include <mia/2d/defines2d.hh>
#include <mia/2d/2DVector.hh>

NS_MIA_BEGIN
/**
   Evaluate the angle between rays ray_a and ray_b, returned values are in 
   radians, clockwise. 
   \param ray_a 
   \param ray_b
 */
float EXPORT_2D angle(const C2DFVector& ray_a, const C2DFVector& ray_b); 

/**
   Evaluate the angle between rays s-a and s-b, returned values are in 
   radians, clockwise. 
   \param a 
   \param b
   \param s 
 */
float EXPORT_2D angle(const C2DFVector& a, const C2DFVector& b, const C2DFVector& s); 

NS_MIA_END
#endif
