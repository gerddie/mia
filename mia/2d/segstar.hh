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

#ifndef SegStar_h
#define SegStar_h

#include <mia/2d/segpoint.hh>
#include <vector>

NS_MIA_BEGIN

/**
   @ingroup perf 
   \brief Helper class for the segmentation of the left heart ventricle myocardium 

   This class implements the segmentation helper that defines the
   center of a circle, six rays and the circle. 
   It is used to help the (manual) segmentation of the left heart myocardium.
 */

class  EXPORT_2D CSegStar {
public:
	/** Standard constructor */
	CSegStar();

	/**
	   Constructor to create the CSegStar from given data 
	   @param center center of the circle appoximating the outer wall of the myocardium 
	   @param r radius of the circle appoximating the outer wall of the myocardium 
	   @param d1 first ray, its intersection should coinceede with the right ventricle 
	   insertion point 
	   @param d2 second ray direction vector  
	   @param d3 third ray  direction vector 
	 */
	CSegStar(const CSegPoint2D& center, float r, 
		 const CSegPoint2D& d1, const CSegPoint2D& d2, const CSegPoint2D& d3);
	
	/**
	   Constructor to create a CSegStar from a XML sub tree
	   @param node root of the sub tree 
	 */
	CSegStar(const CXMLElement& node);

	/**
	   write the CSegStar info to a XML node 
	   @param node root node to add the info to 
	 */
	void write(CXMLElement& node) const;

	/**
	   Shift the segmentation data 
	   @param delta
	 */
	void shift(const C2DFVector& delta);

	/**
	   Transform the star by transforming the intersections between the rays and the circle
	   and then re-evaluating the center and the circle radius 
	   \param t transformation to be applied 
	 */
	void transform(const C2DTransformation& t);

	/**
	   Transform the star by transforming the intersections between the rays and the circle
	   and then re-evaluating the center and the circle radius 
	   \param t inverse of the transformation to be applied 
	 */
	void inv_transform(const C2DTransformation& t);

	/// center of the circle approximating the outer wall of the LV myocardium 
	CSegPoint2D m_center;
	/// radius of the circle approximating the outer wall of the LV myocardium 
	float m_radius;
	/// the ray directions to define the star 
	CSegPoint2D m_directions[3];
private: 
	void recenter_rays(); 
	void reeval_center(); 
};

NS_MIA_END


#endif
