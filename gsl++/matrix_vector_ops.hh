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


#ifndef gslpp_matrix_vector_ops_hh
#define gslpp_matrix_vector_ops_hh

#include <gsl++/matrix.hh>
#include <gsl++/vector.hh>

namespace gsl {

void multiply_m_m(Matrix& result, const Matrix& lhs, const Matrix& rhs);
void multiply_v_m(DoubleVector& result, const DoubleVector& lhs, const Matrix& rhs); 
void multiply_m_v(DoubleVector& result, const Matrix& rhs, const DoubleVector& lhs);

}


#endif 
