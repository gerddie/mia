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


#ifndef mia_2d_SegPoint_hh
#define mia_2d_SegPoint_hh

namespace xmlpp {
	class Node;
};

#include <mia/2d/2DVector.hh>
#include <mia/2d/defines2d.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN

/**
   Point of a segmentation shape. In addition to be a 2D point it can be 
   read from and written to a XML tree and supports its own tranformation. 
*/

class  EXPORT_2D CSegPoint2D: public C2DFVector {
public:
	CSegPoint2D();

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
	CSegPoint2D(const xmlpp::Node& node);

	/** Write the point as child-node to a given XML tree
	    \param node 
	*/
	void write(xmlpp::Node& node) const;

	/**
	   Tranform the point according to the given tranformation 
	   \param t 
	 */
	void transform(const C2DTransformation& t); 
	/**
	   Approximate the inverse transform of the point by using the given transformation 
	   \param t 
	   \param t 
	 */
	void inv_transform(const C2DTransformation& t); 
};

NS_MIA_END

#endif
