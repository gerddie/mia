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


#ifndef mia_2d_distance_hh
#define mia_2d_distance_hh

#include <vector>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   This function evaluates the 2D distance to a functions defined over a 2D grid
   If the input function is given as raw data that is not the result of another
   call to this distance transform function, then one should call
   distance_transform_prepare to properly prepare the data for the distance transform.

   \param[in] f the squared values of the function to evaluate the distance to
   \returns distances to the function for each grid point
*/
C2DFImage EXPORT_2D distance_transform(const C2DImage& f);

/**
   @ingroup misc

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
