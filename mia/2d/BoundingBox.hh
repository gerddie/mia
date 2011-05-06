/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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


#ifndef __mia_2d_boundingbox_hh
#define __mia_2d_boundingbox_hh

#include <mia/2d/2DVector.hh>
#include <mia/2d/defines2d.hh>


NS_MIA_BEGIN

/// a 2D int vector 
typedef T2DVector<int> C2DIVector;


/**
   \brief 2D axis orthothogonal bounding box. 
 */
class  EXPORT_2D C2DBoundingBox {
public:
	C2DBoundingBox();

	/**
	   Construct a new bounding box with 
	   @param begin left lower corner 
	   @param end right upper corner 
	 */
	C2DBoundingBox(const C2DFVector& begin, const C2DFVector& end);

	/**
	   Combine two bounding boxes 
	   @param other box
	 */
	void unite(const C2DBoundingBox& other);

	/**
	   Change the box to include the given point 
	   @param point 
	 */
	void add(const C2DFVector& point);


	/// @returns left lower grid point 
	C2DIVector get_grid_begin() const;
	
	/// @returns right upper grid point 
	C2DIVector get_grid_end() const;

	/// @returns size of bounding box in the grid  
	C2DBounds get_grid_size() const;

	/// @returns left lower bounding box point 
	C2DFVector get_begin() const;
	
	/// @returns right upper  bounding box point 
	C2DFVector get_end() const;

	/// @returns size of bounding box 
	C2DFVector get_size() const;


	/** enlarge the bounding box by a given amount 
	    @param boundary 
	*/
	void enlarge(float boundary);


	/// @returns true if the box is not empty 
	bool empty() const;

private:
	bool m_empty;
	C2DFVector m_begin;
	C2DFVector m_end;
};

NS_MIA_END

#endif
