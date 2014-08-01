/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef SegSection_h
#define SegSection_h

#include <vector>
#include <string>

#include <mia/2d/segpoint.hh>
#include <mia/2d/boundingbox.hh>
#include <mia/2d/polygon.hh>

NS_MIA_BEGIN

/**
   @ingroup perf 
   \brief Segmentation class to represent a section of the LV myocardium 

   Structure to save the segmentation of a section of the myocardium. 
   In theory this can be used for any king of segmentation that uses 
   a polynom to approximate a segmented 2D shape. 
*/


class  EXPORT_2D CSegSection {
public:
	/// convenicence typedef for the points defining the section 
	typedef std::vector<CSegPoint2D> Points;
	
	/// default constructor 
	CSegSection();

	/**
	   Constructor to create a segmentation shape and naming it 
	   @param id ID of the section (and color identifier) 
	   @param points the points that define a closed polynom representing the shape 
	   @param is_open describes if points should be interpreted as polygon 
	   (i.e. the last point connects to the first), or as poly-line only.
	 */
	CSegSection(const std::string& id, const Points& points, bool is_open);

	/**
	   Constructor to create a segmentation shape based on a XML sub tree 
	   @param node root of the XML sub tree 
	   \param version segmentation set version the node stems from. 
	*/
	CSegSection(xmlpp::Node& node, int version);

	/**
	   Store the segmented section into a XML sub-tree 
	   @param node parent node to which the subtree should be added 
	   \param version segmentation set version the node stems from. 
	*/
	void write(xmlpp::Node& node, int version) const;

	/// \returns the ID of the section 
	const std::string& get_id() const;

	/// \returns the list of the points defining the section shape 
	const Points& get_points()const;

	/// \returns the orthogonal bounding box enclosing the shape 
	const C2DBoundingBox get_boundingbox() const;

	/**
	   translate the segmentation by a given shift 
	   @param delta 
	 */
	void shift(const C2DFVector& delta);

	/**
	   Evaluate the Hausdorff distance between this shape and another one 
	   @param other 
	   @returns the Hausdorff distance
	 */
	float get_hausdorff_distance(const CSegSection& other) const;

	/**
	   Append this shape to another shape, a very crude version of a logical or 
	   @param polygon to add the shape to 
	*/
	void append_to(C2DPolygon& polygon)const;

	/**
	   transform the shape by transforming its individual points 
	   @param t the transformation to be applied 
	 */
	void transform(const C2DTransformation& t); 

	/**
	   transform the shape by transforming its individual points 
	   @param t the inverse of the transformation to be applied 
	 */
	void inv_transform(const C2DTransformation& t); 

	/**
	   Draw the shape to a 2D image with a given color 
	   @param output image to draw to 
	   @param color color to use 
	 */
	void draw(C2DUBImage& output, unsigned char color)const; 

	/**
	   Draw the binary shape to a 2D image by xor-ing with what is already in there 
	   @param output image to draw to 
	*/
	void draw_xor(C2DUBImage& output)const; 

	/// \returns whether the curve is open (true) or closed (false). 
	bool is_open() const; 

private:
	std::string m_id;
	Points m_points;
	bool m_is_open; 
};

NS_MIA_END

#endif
