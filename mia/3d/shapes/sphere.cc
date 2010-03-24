/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2005 - 2010
 *
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
	_M_r(2)
{
	add_parameter("r", new CFloatParameter(_M_r, 0, numeric_limits<float>::max(), false, "sphere radius"));
}

C3DShapePlugin::ProductPtr CSphere3DShapeFactory::do_create()const
{
	return CSphere3DShapeFactory::ProductPtr(new CSphere3DShape(_M_r));
}


const string CSphere3DShapeFactory::do_get_descr()const
{
	return string("spherical shape mask creator");
}

bool CSphere3DShapeFactory::do_test()const
{
	for (size_t l = 1; l < 3; ++l) {

		size_t s = 2 * l +1;
		size_t r2 = l*l;

		CSphere3DShape shape(l);

		C3DShape::Mask mask = shape.get_mask();
		if (mask.get_size().x != mask.get_size().z ||
		    mask.get_size().x != mask.get_size().y ||
		    mask.get_size().x != s) {
			cvfail() << "Sphere shape creator: mask size error\n";
			return false;
		}

		for ( size_t z = 0; z < s; ++z)
			for ( size_t y = 0; y < s; ++y)
				for ( size_t x = 0; x < s; ++x) {
					float r_h = (x - l) * (x - l) + (y - l) * (y -l) + (z - l) * (z -l);
					if (mask(x,y,z) && r_h > r2) {
						cvfail() << get_name() <<"mask point outside radius\n";
						return false;
					}
					if (!mask(x,y,z) && r_h <= r2) {
						cvfail() << get_name() <<"non-mask point inside radius\n";
						return false;
					}
				}
	}
	return true;
}
NS_MIA_END
