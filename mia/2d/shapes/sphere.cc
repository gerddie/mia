/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Evolutionary Anthropoloy
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>


#include "sphere.hh"

NS_MIA_BEGIN

using namespace std;

class CSphere2DShape: public C2DShape {
public:
	CSphere2DShape(float radius);
};

CSphere2DShape::CSphere2DShape(float radius)
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
	add_parameter("r", new CFloatParameter(m_r, 0, numeric_limits<float>::max(), false, "sphere radius"));
}

C2DShapePlugin::ProductPtr CSphere2DShapeFactory::do_create()const
{
	return CSphere2DShapeFactory::ProductPtr(new CSphere2DShape(m_r));
}


const string CSphere2DShapeFactory::do_get_descr()const
{
	return string("spherical shape mask creator");
}

bool CSphere2DShapeFactory::do_test()const
{
	for (size_t l = 1; l < 3; ++l) {

		size_t s = 2 * l +1;
		size_t r2 = l*l;

		CSphere2DShape shape(l);

		C2DShape::Mask mask = shape.get_mask();
		if (mask.get_size().x != mask.get_size().y ||
		    mask.get_size().x != s) {
			cvfail() << "Sphere shape creator: mask size error\n";
			return false;
		}

		for ( size_t y = 0; y < s; ++y)
			for ( size_t x = 0; x < s; ++x) {
				float r_h = (x - l) * (x - l) + (y - l) * (y -l);
				if (mask(x,y) && r_h > r2) {
					cvfail() << get_name() <<"mask point outside radius\n";
					return false;
				}
				if (!mask(x,y) && r_h <= r2) {
					cvfail() << get_name() <<"non-mask point inside radius\n";
					return false;
				}
			}
	}
	return true;
}

NS_MIA_END
