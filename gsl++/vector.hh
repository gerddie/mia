/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef GSLPP_VECTOR_HH
#define GSLPP_VECTOR_HH

#include <gsl++/vector_template.hh>

namespace gsl {

typedef TVector<double>  DoubleVector; 
typedef TVector<float>   FloatVector; 

typedef TVector<long>    LongVector; 
typedef TVector<int>     IntVector; 
typedef TVector<short>   ShortVector; 
typedef TVector<char>    CharVector; 

typedef TVector<ulong>   ULongVector; 
typedef TVector<uint>    UIntVector; 
typedef TVector<ushort>  UShortVector; 
typedef TVector<uchar>   UCharVector; 

}

#endif


