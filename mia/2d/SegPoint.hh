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

class  EXPORT_2D CSegPoint2D: public C2DFVector {
public:
	CSegPoint2D();
	CSegPoint2D(float x, float y);
	CSegPoint2D(const xmlpp::Node& node);
	void write(xmlpp::Node& node) const;
	void transform(const C2DTransformation& t); 
	void inv_transform(const C2DTransformation& t); 
};

NS_MIA_END

#endif
