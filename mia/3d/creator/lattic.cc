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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <limits>
#include <mia/core/type_traits.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/creator/lattic.hh>

NS_BEGIN(creator_lattic_3d);
using namespace mia;
using namespace std;
using namespace boost;

C3DLatticCreator::C3DLatticCreator(const C3DFVector& freq):
	m_freq(2*M_PI/ freq.x,2*M_PI/ freq.y, 2*M_PI/ freq.z)
{
}

P3DImage C3DLatticCreator::operator () (const C3DBounds& size, EPixelType type) const
{
	C3DFImage helper = do_create(size);
	
	string convert_descr = string("convert:repn=") + CPixelTypeDict.get_name(type); 
	auto converter = C3DFilterPluginHandler::instance().produce(convert_descr); 

	return converter->filter(helper); 
}

template <typename T, bool is_float>
struct move_range {
	static T apply(double x) {
		const double range = numeric_limits<T>::max() - numeric_limits<T>::min();
		return T( (0.5 * x + 0.5) * range + numeric_limits<T>::min());
	}
};

template <typename T>
struct move_range<T, true> {
	static T apply(double x) {
		return x;
	}
};

C3DFImage C3DLatticCreator::do_create(const C3DBounds& size) const
{
	C3DFImage hresult = C3DFImage(size);
	auto p = hresult.begin();

	T3DVector<double> center(size.x / 2.0, size.y / 2.0, size.z / 2.0);
	T3DVector<double> rmax = center / 2;
	rmax.x *= rmax.x;
	rmax.y *= rmax.y;
	rmax.z *= rmax.z;

	for (size_t z = 0; z < size.z; ++z) {
		double dz = center.z - z;
		dz  *= dz;
		double ez = exp(-dz/rmax.z);
		double vz = sin(m_freq.z * z); 
		for (size_t y = 0; y < size.y; ++y) {
			double dy = center.y - y;
			dy *= dy;
			double ey = exp(-dy/rmax.y) * ez; 
			double vy = sin(m_freq.y * y) + vz; 
			for (size_t x = 0; x < size.x; ++x, ++p) {
				double dx = center.x - x;
				dx *= dx;
				*p = (sin(m_freq.x * x) + vy) * exp(-dx/rmax.x) * ey; 
			}
		}
	}
	
	return  hresult;
}

C3DLatticCreatorPlugin::C3DLatticCreatorPlugin():
	C3DImageCreatorPlugin("lattic"),
	m_freq(16.0, 16.0, 16.0)
{
	add_parameter("fx", make_ci_param(m_freq.x, 1, 64, false, "frequency in x-dir"));
	add_parameter("fy", make_ci_param(m_freq.y, 1, 64, false, "frequency in y-dir"));
	add_parameter("fz", make_ci_param(m_freq.z, 1, 64, false, "frequency in z-dir"));
}

C3DImageCreator *C3DLatticCreatorPlugin::do_create()const
{
	return new C3DLatticCreator(m_freq);
}

const string C3DLatticCreatorPlugin::do_get_descr()const
{
	return "3D lattic creation program";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLatticCreatorPlugin();
}

NS_END
