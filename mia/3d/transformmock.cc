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

#define VSTREAM_DOMAIN "transformmock"

#include <mia/3d/transformmock.hh>

NS_MIA_BEGIN

C3DTransformMock::C3DTransformMock():
       C3DTransformation(C3DInterpolatorFactory("bspline:d=0", "mirror"))
{
}

C3DTransformMock::C3DTransformMock(const C3DBounds& size, const C3DInterpolatorFactory& ipf):
       C3DTransformation(ipf),
       m_size(size)
{
}

C3DTransformation *C3DTransformMock::do_clone() const
{
       return NULL;
}

C3DTransformation *C3DTransformMock::invert() const
{
       return NULL;
}

P3DTransformation C3DTransformMock::do_upscale(const C3DBounds&) const
{
       return P3DTransformation();
}

void C3DTransformMock::update(float, const C3DFVectorfield&)
{
}

size_t C3DTransformMock::degrees_of_freedom() const
{
       return 3 * m_size.x * m_size.y * m_size.z;
}

void C3DTransformMock::set_identity()
{
}

C3DFMatrix C3DTransformMock::derivative_at(const C3DFVector& MIA_PARAM_UNUSED(x)) const
{
       return C3DFMatrix(C3DFVector(1.0, 2.0, 3.0),
                         C3DFVector(4.0, 5.0, 6.0),
                         C3DFVector(7.0, 8.0, 9.0));
}

C3DFMatrix C3DTransformMock::derivative_at(int, int, int) const
{
       return C3DFMatrix(C3DFVector(1.0, 2.0, 3.0),
                         C3DFVector(4.0, 5.0, 6.0),
                         C3DFVector(7.0, 8.0, 9.0));
}

void C3DTransformMock::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
       auto r = params.begin();

       for (auto f = gradient.begin(); f != gradient.end(); ++f) {
              *r++ = f->x;
              *r++ = f->y;
              *r++ = f->z;
       }
}

float C3DTransformMock::get_max_transform() const
{
       return sqrt(1.2 * 1.2 +  2.3 * 2.3 + 4.1 * 4.1);
}

const C3DBounds& C3DTransformMock::get_size() const
{
       return m_size;
}

CDoubleVector  C3DTransformMock::get_parameters() const
{
       CDoubleVector result(degrees_of_freedom());

       if (degrees_of_freedom() >= 3) {
              result[2] = -4.1;
              result[1] = -2.3;
              result[0] = -1.2;
       }

       return result;
}

void C3DTransformMock::set_parameters(const CDoubleVector& /*params*/)
{
}


float C3DTransformMock::pertuberate(C3DFVectorfield&) const
{
       return 1.1;
}

C3DFVector C3DTransformMock::get_displacement_at(const C3DFVector& x) const
{
       if (x.x == 0.0 && x.y == 0.0)
              return C3DFVector(-1.2, -2.3, -4.3);
       else
              return C3DFVector();
}

C3DFVector C3DTransformMock::operator () (const C3DFVector& x) const
{
       return x - get_displacement_at(x);
}

float C3DTransformMock::get_jacobian(const C3DFVectorfield&, float delta) const
{
       return delta;
}

P3DImage C3DTransformMock::do_transform(const C3DImage& src, const C3DInterpolatorFactory&) const
{
       return P3DImage(src.clone());
}

float C3DTransformMock::divergence() const
{
       return 1.0f;
}

float C3DTransformMock::curl() const
{
       return 2.0f;
}

double C3DTransformMock::get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const
{
       gradient[0] = wd;
       gradient[1] = wr;
       gradient[2] = wr - wd;
       return wd + wr;
}

double C3DTransformMock::get_divcurl_cost(double wd, double wr) const
{
       return wd + wr;
}


C3DTransformation::const_iterator C3DTransformMock::begin() const
{
       return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0, 0, 0), m_size));
}

C3DTransformation::const_iterator C3DTransformMock::end() const
{
       return C3DTransformation::const_iterator(new iterator_impl(m_size, m_size));
}

C3DTransformation::const_iterator C3DTransformMock::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
       return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0, 0, 0), begin, end, m_size));
}

C3DTransformation::const_iterator C3DTransformMock::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
       return C3DTransformation::const_iterator(new iterator_impl(m_size, begin, end, m_size));
}


C3DTransformation::iterator_impl *C3DTransformMock::iterator_impl::clone()const
{
       return new C3DTransformMock::iterator_impl(get_pos(), get_size());
}

C3DTransformMock::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size):
       C3DTransformation::iterator_impl(pos, size),
       m_value(-1.2, -2.3, -4.3)
{
}

C3DTransformMock::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin,
              const C3DBounds& end, const C3DBounds& size):
       C3DTransformation::iterator_impl(pos, begin, end, size),
       m_value(-1.2, -2.3, -4.3)
{
}

const C3DFVector& C3DTransformMock::iterator_impl::do_get_value()const
{
       return m_value;
}

void C3DTransformMock::iterator_impl::do_x_increment()
{
       m_value = C3DFVector(get_pos());
}

void C3DTransformMock::iterator_impl::do_y_increment()
{
       m_value = C3DFVector(get_pos());
}

void C3DTransformMock::iterator_impl::do_z_increment()
{
       m_value = C3DFVector(get_pos());
}




NS_MIA_END
