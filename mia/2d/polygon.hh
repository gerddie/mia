/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_2d_polygon_hh
#define mia_2d_polygon_hh

#include <vector>
#include <mia/2d/vector.hh>
#include <mia/2d/defines2d.hh>



NS_MIA_BEGIN

/**
   @ingroup misc 
   \brief A simple 2D polygon class.  

   This class defines a polygon in 2D space and some functions on it. 
   @remark Adding two polygons together is not done in a proper algebraic way.
 */
class EXPORT_2D C2DPolygon {
public:

	/**
	   Evaluate if a given points is within the polygon if the 
	   polygon is considered to be a closed set, i.e. contains its boundary. 
	   @param point 
	   @returns true if the poin is inside, false otherwiese  
	 */
	bool is_inside_closed_set(const C2DFVector& point) const;

	/**
	   Evaluate if a given points is within the polygon if the 
	   polygon is considered to be an open set, i.e. doesn't contain its boundary. 
	   @param point 
	   @returns true if the poin is inside, false otherwiese  
	 */

	bool is_inside_open_set(const C2DFVector& point) const;


	/**
	   Append a point to the polyon 
	 */
	void append(const C2DFVector& point);

	/**
	   @returns the number of points within the polygon 
	 */
	size_t size() const;

	/**
	   Evaluate the minimum distance of a point to the polygon 
	   @param point 
	   @returns distance 
	 */
	float get_mimimum_distance(const C2DFVector& point)const ;

	/**
	   Evaluate the Hausdorff distance between this polygon and  another one 
	   @param other 
	   @returns Hausdorff distance 
	 */
	float get_hausdorff_distance(const C2DPolygon& other)const ;
 private:

	bool is_on_line(const C2DFVector& p, const C2DFVector& a, const C2DFVector& b) const;

	std::vector<C2DFVector> m_points;
};

/// Pointer type for 2D polygons 
typedef std::shared_ptr<C2DPolygon > P2DPolygon;

NS_MIA_END

#endif
