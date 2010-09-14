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


#ifndef SegSection_h
#define SegSection_h

#include <vector>
#include <string>

#include <mia/2d/SegPoint.hh>
#include <mia/2d/BoundingBox.hh>
#include <mia/2d/polygon.hh>

NS_MIA_BEGIN



class  EXPORT_2D CSegSection {
public:
	typedef std::vector<CSegPoint2D> Points;
	CSegSection();
	CSegSection(const std::string& id, const Points& points);
	CSegSection(xmlpp::Node& node);
	void write(xmlpp::Node& node) const;
	const std::string& get_id() const;
	const Points& get_points()const;
	const C2DBoundingBox get_boundingbox() const;
	void shift(const C2DFVector& delta);
	float get_hausdorff_distance(const CSegSection& other) const;
	void append_to(C2DPolygon& polygon)const;
	void transform(const C2DTransformation& t); 
	void inv_transform(const C2DTransformation& t); 
	void draw(C2DUBImage& mask, unsigned char color)const; 
private:
	std::string m_id;
	Points m_points;
};

NS_MIA_END

#endif
