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

#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>

#include <mia/3d/quaternion.hh>
#include <mia/core/utils.hh>
#include <mia/core/msgstream.hh>
#include <vector>
#include <memory>


NS_MIA_BEGIN
using std::swap;
using std::unique_ptr;
using std::invalid_argument;

Quaternion::Quaternion():
       m_w(0.0)
{
}

Quaternion::Quaternion(const C3DFMatrix& m): Quaternion(C3DDMatrix(m))
{
}

Quaternion::Quaternion(const C3DDMatrix& m)
{
       const double m00 = (m.x.x - m.y.y - m.z.z) / 3.0;
       const double m01 = (m.y.x + m.x.y) / 3.0;
       const double m02 = (m.z.x + m.x.z) / 3.0;
       const double m03 = (m.z.y - m.y.z) / 3.0;
       const double m11 = (m.y.y - m.x.x - m.z.z) / 3.0;
       const double m12 = (m.z.y + m.y.z) / 3.0;
       const double m13 = (m.x.z - m.z.x) / 3.0;
       const double m22 = (m.z.z - m.x.x - m.y.y) / 3.0;
       const double m23 = (m.y.x - m.x.y) / 3.0;
       const double m33 = (m.z.z + m.x.x + m.y.y) / 3.0;
       double data[16] = {
              m00, m01, m02, m03,
              m01, m11, m12, m13,
              m02, m12, m22, m23,
              m03, m13, m23, m33
       };
       gsl_matrix_view gslm  = gsl_matrix_view_array (data, 4, 4);
       auto gsl_vector_delete = [](gsl_vector * p) {
              gsl_vector_free(p);
       };
       unique_ptr<gsl_vector, decltype(gsl_vector_delete)> eval(gsl_vector_alloc (4), gsl_vector_delete );
       auto gsl_matrix_delete = [](gsl_matrix * p) {
              gsl_matrix_free(p);
       };
       unique_ptr<gsl_matrix, decltype(gsl_matrix_delete)> evec(gsl_matrix_alloc (4, 4), gsl_matrix_delete );
       auto gsl_eigen_symmv_delete = [](gsl_eigen_symmv_workspace * p) {
              gsl_eigen_symmv_free(p);
       };
       unique_ptr<gsl_eigen_symmv_workspace, decltype(gsl_eigen_symmv_delete)> ws(gsl_eigen_symmv_alloc (4), gsl_eigen_symmv_delete);
       gsl_eigen_symmv (&gslm.matrix, eval.get(), evec.get(), ws.get());
       gsl_eigen_symmv_sort (eval.get(), evec.get(), GSL_EIGEN_SORT_ABS_ASC);

       if (cverb.get_level() <= vstream::ml_debug ) {
              for (int i = 0; i < 4; ++i) {
                     cvdebug() << "eval [" << i << "] = " << gsl_vector_get(eval.get(), i) << "\n";
                     cvdebug() << "evec [" << i << "] = ";

                     for (int j = 0; j < 4; ++j) {
                            cverb << gsl_matrix_get(evec.get(), j, i) << ", ";
                     }

                     cverb << "\n\n";
              }
       }

       m_v.x = gsl_matrix_get(evec.get(), 0, 3);
       m_v.y = gsl_matrix_get(evec.get(), 1, 3);
       m_v.z = gsl_matrix_get(evec.get(), 2, 3);
       m_w   = gsl_matrix_get(evec.get(), 3, 3);

       // technically, this is not necessary, but for testing it is better
       if (m_w < 0.0) {
              m_v *= -1.0f;
              m_w = -m_w;
       }
}

Quaternion::Quaternion(const C3DDVector& rot):
       m_v(0.0, 0.0, 0.0),
       m_w(1.0)

{
       double cos_phi, sin_phi, cos_psi, sin_psi, cos_theta, sin_theta;
       sincos(0.5 * rot.x, &sin_phi,   &cos_phi);
       sincos(0.5 * rot.y, &sin_theta, &cos_theta);
       sincos(0.5 * rot.z, &sin_psi,   &cos_psi);
       m_w   = cos_phi * cos_theta * cos_psi + sin_phi * sin_theta * sin_psi;
       m_v.x = sin_phi * cos_theta * cos_psi - cos_phi * sin_theta * sin_psi;
       m_v.y = cos_phi * sin_theta * cos_psi + sin_phi * cos_theta * sin_psi;
       m_v.z = cos_phi * cos_theta * sin_psi - sin_phi * sin_theta * cos_psi;
}

const Quaternion Quaternion::_1(1, 0, 0, 0);

#if 0
Quaternion::Quaternion(const C3DFMatrix& rot)
{
       double q4  = 1 + rot.x.x + rot.y.y + rot.z.z;

       if (q4 <= 0) {
              throw create_exception<invalid_argument>("Quaternion: Matrix ", rot,
                            " is not evem close to be a rotation matrix.");
       }

       m_w = 0.5 * sqrt(q4);
       double s = 0.25 / m_w;
       m_v.x = s * (rot.z.y - rot.y.z);
       m_v.y = s * (rot.x.z - rot.z.x);
       m_v.z = s * (rot.y.x - rot.x.y);
}
#endif

#if 0
const C3DFMatrix Quaternion::get_rotation_matrix() const
{
       const C3DDMatrix qq(m_v.x * m_v, m_v.y * m_v, m_v.z * m_v);
       cvdebug() << qq << "\n";
       const C3DDMatrix Q(C3DDVector(0, -m_v.z, m_v.y),
                          C3DDVector(m_v.z, 0, -m_v.x),
                          C3DDVector(-m_v.y, m_v.x, 0));
       cvdebug() << Q << "\n";
       cvdebug() << (m_w * m_w - m_v.norm2()) * C3DDMatrix::_1 << "\n";
       const C3DDMatrix r = (m_w * m_w - m_v.norm2()) * C3DDMatrix::_1 + 2.0 * qq + 2.0 * m_w * Q;
       cvdebug() << r << "\n";
       return C3DFMatrix(r);
}
#endif

Quaternion::Quaternion(double w, double  x, double y, double z):
       m_v(x, y, z),
       m_w(w)
{
}

double Quaternion::norm() const
{
       return sqrt(m_v.norm2() + m_w * m_w);
}

void Quaternion::normalize()
{
       double n = norm();

       if (n > 0) {
              m_v /= n;
              m_w /= n;
       }
}

C3DDVector Quaternion::get_euler_angles() const
{
       FUNCTION_NOT_TESTED;
       const double sqw = m_w   * m_w;
       const double sqx = m_v.x * m_v.x;
       const double sqy = m_v.y * m_v.y;
       const double sqz = m_v.z * m_v.z;
       const double norm2 = sqx + sqy + sqz + sqw;
       const double test = m_v.x * m_v.y + m_v.z * m_w;

       if (test > 0.499 * norm2)
              return C3DDVector(2 * atan2(m_v.x, m_w), M_PI / 2.0, 0.0);

       if (test < -0.499 * norm2)
              return C3DDVector(-2 * atan2(m_v.x, m_w), -M_PI / 2.0, 0.0);

       return C3DDVector(atan2(2.0 * (m_v.y * m_w - m_v.x * m_v.z), sqx - sqy - sqz + sqw),
                         asin(2 * test / norm2),
                         atan2(2.0 * (m_v.x * m_w - m_v.y * m_v.z), -sqx + sqy - sqz + sqw));
}

Quaternion Quaternion::inverse() const
{
       return Quaternion(m_w, -m_v.x, -m_v.y, -m_v.z);
}


const C3DDMatrix Quaternion::get_rotation_matrix() const
{
       const double a2 = m_w   * m_w;
       const double b2 = m_v.x * m_v.x;
       const double c2 = m_v.y * m_v.y;
       const double d2 = m_v.z * m_v.z;
       const double bc = 2.0 * m_v.x * m_v.y;
       const double ad = 2.0 * m_w * m_v.z;
       const double bd = 2.0 * m_v.x * m_v.z;
       const double ac = 2.0 * m_w * m_v.y;
       const double cd = 2.0 * m_v.y * m_v.z;
       const double ab = 2.0 * m_w * m_v.x;
       return C3DDMatrix(C3DDVector(a2 + b2 - c2 - d2, bc - ad, bd + ac),
                         C3DDVector(bc + ad, a2 - b2 + c2 - d2, cd - ab),
                         C3DDVector(bd - ac, cd + ab, a2 - b2 - c2 + d2));
}



void Quaternion::print(std::ostream& os) const
{
       os << m_w << "," << m_v;
}

EXPORT_3D std::istream& operator >> (std::istream& is, Quaternion& a)
{
       const char *msg = "Unable to read quaternion from stream";
       float w;
       C3DFVector v;
       is >> w;
       eat_char(is, ',', msg);
       is >> v;
       a = Quaternion(w, v.x, v.y, v.z);
       return is;
}

bool operator == (const Quaternion& a, const Quaternion& b)
{
       return (a.m_v == b.m_v) && (a.m_w == b.m_w);
}

bool operator != (const Quaternion& a, const Quaternion& b)
{
       return !(a == b);
}

Quaternion& Quaternion::operator += (const Quaternion& other)
{
       m_v += other.m_v;
       m_w += other.m_w;
       return *this;
}

Quaternion& Quaternion::operator -= (const Quaternion& other)
{
       m_v -= other.m_v;
       m_w -= other.m_w;
       return *this;
}


Quaternion& Quaternion::operator *= (const Quaternion& o)
{
       const double a = m_w * o.m_w - m_v.x * o.m_v.x - m_v.y * o.m_v.y - m_v.z * o.m_v.z;
       const double b = m_v.x * o.m_w + m_w * o.m_v.x + m_v.y * o.m_v.z - m_v.z * o.m_v.y;
       const double c = m_w  * o.m_v.y - m_v.x * o.m_v.z + m_v.y * o.m_w + m_v.z * o.m_v.x;
       const double d = m_w * o.m_v.z + m_v.x * o.m_v.y - m_v.y * o.m_v.x + m_v.z * o.m_w;
       m_w = a;
       m_v = C3DFVector(b, c, d);
       return *this;
}

NS_MIA_END

