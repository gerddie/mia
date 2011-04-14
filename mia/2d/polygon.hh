/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#ifndef mia_2d_polygon_hh
#define mia_2d_polygon_hh

#include <vector>
#include <mia/core/shared_ptr.hh>

#include <mia/2d/2DVector.hh>
#include <mia/2d/defines2d.hh>



NS_MIA_BEGIN

/**
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
