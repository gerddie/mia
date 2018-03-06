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

#include <mia/3d/trackpoint.hh>
#include <mia/template/cvd_io_trait.hh>
#include <mia/template/trackpoint.cxx>


NS_MIA_BEGIN
using namespace std;

template <>
struct NDVectorIOcvd<C3DFVector> {
       static bool read(istream& is, C3DFVector& value)
       {
              char c;
              is >> value.x;
              is >> c;

              if (c != ';')
                     return false;

              is >> value.y;
              is >> c;

              if (c != ';')
                     return false;

              is >> value.z;
              return true;
       }

       static void write(ostream& os, const C3DFVector& value)
       {
              os << value.x << ";" << value.y << ";" << value.z;
       }
};

template class  TTrackPoint<C3DTransformation>;

NS_MIA_END
