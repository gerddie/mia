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

#ifndef SegStar_h
#define SegStar_h

#include <mia/2d/SegPoint.hh>
#include <vector>

NS_MIA_BEGIN

class  EXPORT_2D CSegStar {
public:
	CSegStar();
	CSegStar(const CSegPoint2D& center, float r, const CSegPoint2D& d1, const CSegPoint2D& d2, const CSegPoint2D& d3);
	CSegStar(const xmlpp::Node& node);

	void write(xmlpp::Node& node) const;

	void shift(const C2DFVector& delta);

	void transform(const C2DTransformation& t);

	CSegPoint2D m_center;
	float m_radius;
	CSegPoint2D m_directions[3];
};

NS_MIA_END


#endif
