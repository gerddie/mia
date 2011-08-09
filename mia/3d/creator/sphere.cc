/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


/*
  LatexBeginPluginDescription{3D image creators}

  \subsection{Sphere Creator}
  \label{creator3d:sphere}
   
  \begin{description}
   
  \item [Plugin:] sphere
  \item [Description:] Creates an image with a filled distorted sphere-like shape. The shape and filling 
       patternare given by the parameters. 
   
   \plugtabstart
   f & float & spherical change frequency & 1.0 \\
   p & float & spherical shape parameter (2.0 = circle)& 2.0 \\
   \plugtabend
   
   \end{description}


  LatexEnd
*/


#define VSTREAM_DOMAIN "SHAPE SPHERE"
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <limits>
#include <mia/core/type_traits.hh>
#include <mia/3d/creator.hh>

NS_BEGIN(creator_sphere_3d);
using namespace mia;
using namespace std;
using namespace boost;

class C3DSphereCreator	: public C3DImageCreator {
public:
	C3DSphereCreator(float f, float p);
	virtual P3DImage operator () (const C3DBounds& size, EPixelType type) const;
private:
	template <typename T>
 	P3DImage do_create(const C3DBounds& size) const;
	float m_f;
	double m_p;
};

C3DSphereCreator::C3DSphereCreator(float f, float p):
	m_f(f),
	m_p(p)
{
}

P3DImage C3DSphereCreator::operator () (const C3DBounds& size, EPixelType type) const
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
		return P3DImage();
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
P3DImage C3DSphereCreator::do_create(const C3DBounds& size) const
{
	T3DImage<T> *result = new T3DImage<T>(size);
	typename T3DImage<T>::iterator p = result->begin();

	T3DVector<double> center(size.x / 2.0, size.y / 2.0, size.z / 2.0);
	T3DVector<double> rmax = center / 2;
	rmax.x = pow(rmax.x, m_p);
	rmax.y = pow(rmax.y, m_p);
	rmax.z = pow(rmax.z, m_p);

	double l = rmax.x + rmax.y + rmax.z;

	const bool is_float = is_floating_point<T>::value;

	for (size_t z = 0; z < size.z; ++z) {
		double dz = abs(center.z - z);
		dz  = pow(dz, m_p);
		for (size_t y = 0; y < size.y; ++y) {
			double dy = abs(center.y - y);
			dy  = pow(dy, m_p);

			dy += dz;
			for (size_t x = 0; x < size.x; ++x, ++p) {
				double dx = abs(center.x - x);
				dx = pow(dx, m_p);

				dx += dy;
				*p =  (dx > l) ? 0 : *p = move_range<T,is_float>::apply(cos( dx / l * m_f * M_PI ));
			}
		}
	}

	return  P3DImage(result);
}


class C3DSphereCreatorPlugin : public  C3DImageCreatorPlugin {
public:
	C3DSphereCreatorPlugin();
private:
	virtual C3DImageCreator *do_create()const;
	virtual const string do_get_descr()const;
	virtual bool do_test() const;
	float m_f;
	float m_p;
};

C3DSphereCreatorPlugin::C3DSphereCreatorPlugin():
	C3DImageCreatorPlugin("sphere"),
	m_f(2.0),
	m_p(2.0)
{
	add_parameter("f", new CFloatParameter(m_f, 0, 10, false, "spherical change frequency"));
	add_parameter("p", new CFloatParameter(m_p, 0.1, 100, false, "spherical shape parameter (2.0 = sphere)"));
}

C3DImageCreator *C3DSphereCreatorPlugin::do_create()const
{
	return new C3DSphereCreator(m_f, m_p);
}

const string C3DSphereCreatorPlugin::do_get_descr()const
{
	return "3D sphere creation program";
}

bool C3DSphereCreatorPlugin::do_test() const
{
	cvwarn() << "no test implemented\n";
	return true;
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSphereCreatorPlugin();
}

NS_END
