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

#include <iostream>
#include <cmath>

#include <mia/3d/critical_point.hh>

NS_MIA_BEGIN
using namespace std;

C3DCriticalPoint::C3DCriticalPoint(float x, float y, float z):
       gamma(0), x0(x, y, z)
{
}

C3DCriticalPoint::C3DCriticalPoint():
       gamma(0), x0(0, 0, 0)
{
}

C3DCriticalPoint::C3DCriticalPoint(const C3DFVector& _x0):
       gamma(0), x0(_x0)
{
}

C3DCriticalPoint::C3DCriticalPoint(const C3DCriticalPoint& org):
       gamma(org.gamma), x0(org.x0), A(org.A)
{
}

C3DCriticalPoint&  C3DCriticalPoint::operator = (const C3DCriticalPoint& org)
{
       if (this == &org) {
              return *this;
       }

       gamma = org.gamma;
       x0 = org.x0;
       A = org.A;
       return *this;
}

C3DFVector C3DCriticalPoint::at(const C3DFVector& x)const
{
       C3DFVector delta = x - x0;
       float tmp = delta.norm2();

       if (tmp > .0001) {
              return (gamma / tmp) * (A * delta);
       } else {
              //in the eye of the hurrican there is silence
              return C3DFVector();
       }
}

C3DFVector C3DCriticalPoint::at_alt(const C3DFVector& x) const
{
       C3DFVector delta = x - x0;
       float tmp = std::fabs(delta.norm() - gamma);
       return (A * delta) / float( tmp + 1.0);
}

C3DCriticalPointEigen::C3DCriticalPointEigen(C3DCriticalPoint cp):
       location(cp.get_point()),
       portrait(cp.get_a())
{
       estimate();
}

C3DCriticalPointEigen::C3DCriticalPointEigen(const C3DFVector& l, const C3DFMatrix& m):
       location(l),
       portrait(m)
{
       if (!estimate())
              throw range_error("cp-ev estimate failed");
}

C3DCriticalPointEigen::C3DCriticalPointEigen(const C3DFVector& _location,
              const C3DFMatrix& _portrait,
              float _eval1, float _eval2, float _eval3,
              const C3DFVector& _evec1, const C3DFVector& _evec2, const C3DFVector& _evec3,
              EVType _type):
       location(_location),
       portrait(_portrait),
       eval1(_eval1),
       eval2(_eval2),
       eval3(_eval3),
       evec1(_evec1),
       evec2(_evec2),
       evec3(_evec3),
       type(_type)
{
}


bool C3DCriticalPointEigen::estimate()
{
       C3DFVector eval;
       int rank = portrait.get_rank();

       if (rank == 0) {
              type = ev_zero;
              return true;
       }

       int evcalres = portrait.get_eigenvalues(eval);

       if (evcalres == 1 && std::fabs(eval.z) < std::fabs(eval.y) * 0.001) {
              // treat a small complex solution as a real one with two equal evs
              evcalres = 2;
              eval.z = eval.y;
       }

       eval1 = eval.x;
       eval2 = eval.y;
       eval3 = eval.z;

       switch (evcalres) {
       case 3: // three distinct real eigenvalues
              if (rank != 3) {
                     cerr << "ERROR: 3 distinct eigenvalues but rank not 3!" << endl;
                     return false;
              }

              evec1 = portrait.get_complex_eigenvector(0);
              evec2 = portrait.get_complex_eigenvector(1);
              evec3 = portrait.get_complex_eigenvector(2);
              type = ev_real;
              return true;

       case 1: { // one real and two complex eigenvalues
              if (rank != 3) {
                     cerr << "ERROR: 3 distinct eigenvalues but rank not 3!" << endl;
                     return false;
              }

              evec1 = portrait.get_complex_eigenvector(0);
              evec2 = portrait.get_complex_eigenvector(1);
              evec3 = portrait.get_complex_eigenvector(2);
              type = ev_complex;
              return true;
       }

       case 2:// three ev's but at least two are equal
              evec1 = portrait.get_complex_eigenvector(0);
              evec2 = portrait.get_complex_eigenvector(1);
              evec3 = portrait.get_complex_eigenvector(2);
              type = ev_real_two_equal;
              return true;

       case 4:// three real ev's all are equal
              evec1 = portrait.get_complex_eigenvector(0);
              evec2 = portrait.get_complex_eigenvector(1);
              evec3 = portrait.get_complex_eigenvector(2);
              type = ev_real_three_equal;
              return true;

       default:
              evec1 = portrait.get_complex_eigenvector(0);
              return false;
       }

       return true;
}

NS_MIA_END
