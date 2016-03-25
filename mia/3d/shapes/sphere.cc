/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>

#include "sphere.hh"

NS_MIA_BEGIN

using namespace std;

class CSphere3DShape: public C3DShape {
public:
	CSphere3DShape(float radius);
};

CSphere3DShape::CSphere3DShape(float radius)
{
	int hw = static_cast<int>(radius + 1.0);
	float r2 = radius * radius;

	for (int z = -hw; z <= hw; ++z) {
		float z2 = z * z;
		for (int y = -hw; y <= hw; ++y) {
			float y2 = y * y;
			for (int x = -hw; x <= hw; ++x) {
				if (z2 + y2 + x*x <= r2)
					insert(C3DShape::Flat::value_type(x,y,z));
			}
		}
	}
}

CSphere3DShapeFactory::CSphere3DShapeFactory():
	C3DShapePlugin("sphere"),
	m_r(2)
{
	add_parameter("r", make_positive_param(m_r, false, "sphere radius"));
}

C3DShape *CSphere3DShapeFactory::do_create()const
{
	return new CSphere3DShape(m_r);
}


const string CSphere3DShapeFactory::do_get_descr()const
{
	return string("Closed spherical shape neighborhood including the pixels within a given radius r.");
}

NS_MIA_END
