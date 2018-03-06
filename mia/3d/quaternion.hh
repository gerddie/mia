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

#ifndef mia_3d_quaternion_hh
#define mia_3d_quaternion_hh

#include <mia/3d/defines3d.hh>
#include <mia/3d/matrix.hh>
#include <mia/3d/vector.hh>

#include <ostream>
#include <cmath>

NS_MIA_BEGIN


/**
   \ingroup misc
   \brief a class to implement a quaternion

   This class implements some operations of a quaternion.
   The rotation is implemented as clockwise/left-handed rotation.

*/

class EXPORT_3D Quaternion
{

public:
       /**
         The standard constructor that sets all values of the quaternion to zero.
       */
       Quaternion();

       /**
          The copy constructor.
          \param other
        */
       Quaternion(const Quaternion& other) = default;

       /**
          This constructor creates a quaternion from three Euler angles that are applied
          with the <a href="https://secure.wikimedia.org/wikipedia/en/wiki/Conversion_between_quaternions_and_Euler_angles">x-y-z convention</a>.
          \param rot
       */
       Quaternion(const C3DDVector& rot);


       /**
          This constructor creates a quaternion from a 3x3 rotation matrix.
          If mat3x3 is not a true rotation matrix, then this constructor evaluates the
          rotation quaternion that best resembles the matrix transformation.
          \param rot
       */
       Quaternion(const C3DFMatrix& rot);

       /**
          This constructor creates a quaternion from a 3x3 rotation matrix.
          If mat3x3 is not a true rotation matrix, then this constructor evaluates the
          rotation quaternion that best resembles the matrix transformation.
          \param rot
       */
       Quaternion(const C3DDMatrix& rot);

       /**
          Constructor to create a quaternion by directly setting its elements.
        */
       Quaternion(double w, double  x, double y, double z);

       /**
          \param a
          \param b
          \returns true if the quaternions are element-wise equal
        */

       friend bool operator == (const Quaternion& a, const Quaternion& b);

       /// \returns the norm of the quaternion
       double norm() const;

       /// normalizes to quaternion
       void normalize();

       /// \returns the inverse of the quaternion assuming it is normalized
       Quaternion inverse() const;

       /// \returns the Euler angles that correspond to the rotation described by this quaternion
       C3DDVector get_euler_angles() const;

       /**
          in-place adding
          \param other
          \returns reference to the updated quaternion
        */
       Quaternion& operator += (const Quaternion& other);

       /**
          in-place substraction
          \param other
          \returns reference to the updated quaternion
        */
       Quaternion& operator -= (const Quaternion& other);

       /**
          in-place multiplication
          \param other
          \returns reference to the updated quaternion
        */

       Quaternion& operator *= (const Quaternion& other);

       /**
          Print the quaternion to an output stream
          \param os the output stream
        */
       void print(std::ostream& os) const;

       /// \returns the w- or $x_0$ component of the quaternion
       double w() const;

       /// \returns the x- or $x_1$ component of the quaternion
       double x() const;

       /// \returns the y- or $x_2$ component of the quaternion
       double y() const;

       /// \returns the z- or $x_3$ component of the quaternion
       double z() const;

       const C3DDMatrix get_rotation_matrix() const;

       static const Quaternion _1;

private:
       C3DDVector m_v;
       double m_w;
};

bool EXPORT_3D operator == (const Quaternion& a, const Quaternion& b);
bool EXPORT_3D operator != (const Quaternion& a, const Quaternion& b);


inline double Quaternion::w() const
{
       return m_w;
}

inline double Quaternion::x() const
{
       return m_v.x;
}

inline double Quaternion::y() const
{
       return m_v.y;
}

inline double Quaternion::z() const
{
       return m_v.z;
}



inline std::ostream& operator << (std::ostream& os, const Quaternion& a)
{
       a.print(os);
       return os;
}

EXPORT_3D std::istream& operator >> (std::istream& os, Quaternion& a);

NS_MIA_END

#endif
