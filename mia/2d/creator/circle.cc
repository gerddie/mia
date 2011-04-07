/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <limits>
#include <cmath>
#include <mia/core/type_traits.hh>
#include <mia/2d/creator.hh>

NS_BEGIN(creator_circle_2d);
using namespace mia;
using namespace std;
using namespace boost;

class C2DCircleCreator	: public C2DImageCreator {
public:
	C2DCircleCreator(float f, float p);
	virtual P2DImage operator () (const C2DBounds& size, EPixelType type) const;
private:
	template <typename T>
 	P2DImage do_create(const C2DBounds& size) const;
	float m_f;
	double m_p;
};

C2DCircleCreator::C2DCircleCreator(float f, float p):
	m_f(f),
	m_p(p)
{
}

P2DImage C2DCircleCreator::operator () (const C2DBounds& size, EPixelType type) const
{
	switch (type) {
	case it_bit:   return do_create<bool>(size);
	case it_sbyte: return do_create<signed char>(size);
	case it_ubyte: return do_create<unsigned char>(size);
	case it_sshort:return do_create<signed short>(size);
	case it_ushort:return do_create<unsigned short>(size);
	case it_sint:  return do_create<signed int>(size);
	case it_uint:	 return do_create<unsigned int>(size);
#ifdef HAVE_INT64
	case it_slong: return do_create<signed long>(size);
	case it_ulong: return do_create<unsigned long>(size);
#endif
	case it_float: return do_create<float>(size);
	case it_double:return do_create<double>(size);
	default:
		assert(!"unsupported pixel type in image");
		return P2DImage();
	}

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

template <typename T>
P2DImage C2DCircleCreator::do_create(const C2DBounds& size) const
{
	T2DImage<T> *result = new T2DImage<T>(size);
	typename T2DImage<T>::iterator p = result->begin();

	T2DVector<double> center(size.x / 2.0, size.y / 2.0);
	T2DVector<double> rmax = center / 2.0;
	rmax.x = pow(rmax.x, m_p);
	rmax.y = pow(rmax.y, m_p);

	double l = rmax.x + rmax.y;

	const bool is_float = is_floating_point<T>::value;

	for (size_t y = 0; y < size.y; ++y) {
		double dy = center.y - y;
		dy  = pow(dy, m_p);
		for (size_t x = 0; x < size.x; ++x, ++p) {
			double dx = center.x - x;
			dx = pow(dx, m_p);

			dx += dy;
			*p =  (dx > l) ? 0 : *p = move_range<T,is_float>::apply(cos( dx / l * m_f * M_PI ));
		}
	}
	return  P2DImage(result);
}


class C2DCircleCreatorPlugin : public  C2DImageCreatorPlugin {
public:
	C2DCircleCreatorPlugin();
private:
	virtual C2DImageCreatorPlugin::ProductPtr do_create()const;
	virtual const string do_get_descr()const;
	virtual bool do_test() const;
	float m_f;
	float m_p;
};

C2DCircleCreatorPlugin::C2DCircleCreatorPlugin():
	C2DImageCreatorPlugin("circle"),
	m_f(2.0),
	m_p(2.0)
{
	add_parameter("f", new CFloatParameter(m_f, 0, 10, false, "spherical change frequency"));
	add_parameter("p", new CFloatParameter(m_p, 0.1, 100, false, "spherical shape parameter (2.0 = circle)"));
}

C2DImageCreatorPlugin::ProductPtr C2DCircleCreatorPlugin::do_create()const
{
	return C2DImageCreatorPlugin::ProductPtr(new C2DCircleCreator(m_f, m_p));
}

const string C2DCircleCreatorPlugin::do_get_descr()const
{
	return "2D circle creation program";
}

bool C2DCircleCreatorPlugin::do_test() const
{
	cverr() << "C2DCircleCreatorPlugin::do_test not implemented\n";
	return true;
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DCircleCreatorPlugin();
}

NS_END
