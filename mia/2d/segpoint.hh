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

#ifndef mia_2d_SegPoint_hh
#define mia_2d_SegPoint_hh

#include <mia/2d/vector.hh>
#include <mia/2d/defines2d.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN

class CXMLElement; 

/**
   @ingroup perf 
   \brief a point in a 2D segmentation shape 

   Point of a segmentation shape. In addition to be a 2D point it can be 
   read from and written to a XML tree and supports its own tranformation. 
*/

class  EXPORT_2D CSegPoint2D: public C2DFVector {
public:
	CSegPoint2D();

	/// copy constructor 
	CSegPoint2D(const C2DFVector& org);

	/// assignment operator 
	CSegPoint2D& operator = (const C2DFVector& org);

	/**
	   Construct the point with the given coordinates
	   \param x
	   \param y
	 */
	CSegPoint2D(float x, float y);

	
	
        /**
	   Construct the point from a XML node
	   \param node
	 */
	CSegPoint2D(const CXMLElement& node);

	/** Write the point as child-node to a given XML tree
	    \param node 
	*/
	void write(CXMLElement& node) const;

	/**
	   Tranform the point according to the given tranformation 
	   \param t 
	 */
	void transform(const C2DTransformation& t); 
	
	/**
	   Evaluate an approximation of the inverse of the given transform of the point
	   \param t 

	 */
	void inv_transform(const C2DTransformation& t); 
};


template <typename T>
void read_attribute_from_node(const CXMLElement& elm, const std::string& key, T& out_value, bool required); 

void read_attribute_from_node(const CXMLElement& elm, const std::string& key, bool& out_value, bool required); 

NS_MIA_END

#endif
