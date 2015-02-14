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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>


#include "sphere.hh"

NS_MIA_BEGIN

using namespace std;

CSphere2DShapeFactory::CSphere2DShape::CSphere2DShape(float radius)
{
	int hw = static_cast<int>(radius + 1.0);
	float r2 = radius * radius;

	for (int y = -hw; y <= hw; ++y) {
		float y2 = y * y;
		for (int x = -hw; x <= hw; ++x) {
			if (y2 + x*x <= r2)
				insert(C2DShape::Flat::value_type(x,y));
		}
	}
}


CSphere2DShapeFactory::CSphere2DShapeFactory():
	C2DShapePlugin("sphere"),
	m_r(2)
{
	add_parameter("r", make_positive_param(m_r, false, "sphere radius"));
}

C2DShape *CSphere2DShapeFactory::do_create()const
{
	return new CSphere2DShape(m_r);
}


const string CSphere2DShapeFactory::do_get_descr()const
{
	return "Closed spherical neighborhood shape of radius r.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CSphere2DShapeFactory;
}


NS_MIA_END
