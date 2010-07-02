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


#include <sstream>
#include <stdexcept>
#include <cassert>
#include <mia/2d/SegPoint.hh>
#include <mia/core/tools.hh>
#include <libxml++/libxml++.h>

NS_MIA_BEGIN

using namespace xmlpp;
using namespace std;

CSegPoint2D::CSegPoint2D()
{
}

CSegPoint2D::CSegPoint2D(float x, float y):
	C2DFVector(x,y)
{
}

CSegPoint2D::CSegPoint2D(const Node& node)
{
	const Element& elm = dynamic_cast<const Element&>(node);
	Attribute *ax = elm.get_attribute ("x");
	Attribute *ay = elm.get_attribute ("y");
	if (!ax || !ay)
		throw invalid_argument("SegSection:Point attribute x or y not found");
	x = from_string<float>(ax->get_value());
	y = from_string<float>(ay->get_value());
}

void CSegPoint2D::write(Node& node) const
{
	Element* point = node.add_child("point");
	point->set_attribute("y", to_string<float>(y));
	point->set_attribute("x", to_string<float>(x));
}

NS_MIA_END
