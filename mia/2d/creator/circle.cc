/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <limits>
#include <cmath>
#include <mia/2d/creator/circle.hh>

NS_BEGIN(creator_circle_2d);
using namespace mia;
using namespace std;

C2DCircleCreator::C2DCircleCreator(float f, float p):
       m_f(f),
       m_p(p)
{
}

P2DImage C2DCircleCreator::operator () (const C2DBounds& size, EPixelType type) const
{
       switch (type) {
       case it_bit:
              return do_create<bool>(size);

       case it_sbyte:
              return do_create<signed char>(size);

       case it_ubyte:
              return do_create<unsigned char>(size);

       case it_sshort:
              return do_create<signed short>(size);

       case it_ushort:
              return do_create<unsigned short>(size);

       case it_sint:
              return do_create<signed int>(size);

       case it_uint:
              return do_create<unsigned int>(size);
#ifdef HAVE_INT64

       case it_slong:
              return do_create<signed long>(size);

       case it_ulong:
              return do_create<unsigned long>(size);
#endif

       case it_float:
              return do_create<float>(size);

       case it_double:
              return do_create<double>(size);

       default:
              assert(!"unsupported pixel type in image");
              return P2DImage();
       }
}

template <typename T, bool is_float>
struct move_range {
       static T apply(double x)
       {
              const double range = numeric_limits<T>::max() - numeric_limits<T>::min();
              return T( (0.5 * x + 0.5) * range + numeric_limits<T>::min());
       }
};

template <typename T>
struct move_range<T, true> {
       static T apply(double x)
       {
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
              double dy = abs(center.y - y);
              dy  = pow(dy, m_p);

              for (size_t x = 0; x < size.x; ++x, ++p) {
                     double dx = abs(center.x - x);
                     dx = pow(dx, m_p);
                     dx += dy;
                     *p =  (dx > l) ? 0 : move_range<T, is_float>::apply(cos( dx / l * m_f * M_PI ));
              }
       }

       return  P2DImage(result);
}



C2DCircleCreatorPlugin::C2DCircleCreatorPlugin():
       C2DImageCreatorPlugin("circle"),
       m_f(1.0),
       m_p(2.0)
{
       add_parameter("f", make_ci_param(m_f, 0.0f, 10.0f, false, "spherical change frequency"));
       add_parameter("p", make_ci_param(m_p, 0.1f, 100.0f, false, "spherical shape parameter (2.0 = circle)"));
}

C2DImageCreator *C2DCircleCreatorPlugin::do_create()const
{
       return new C2DCircleCreator(m_f, m_p);
}

const string C2DCircleCreatorPlugin::do_get_descr()const
{
       return "2D circle creation program";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DCircleCreatorPlugin();
}

NS_END
