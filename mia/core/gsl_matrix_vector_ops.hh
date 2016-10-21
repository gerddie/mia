/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/gsl_matrix.hh>
#include <mia/core/gsl_vector.hh>

namespace gsl {

void EXPORT_GSL multiply_m_m(Matrix& result, const Matrix& lhs, const Matrix& rhs);
void EXPORT_GSL multiply_mT_m(Matrix& result, const Matrix& lhs, const Matrix& rhs);
void EXPORT_GSL multiply_m_mT(Matrix& result, const Matrix& lhs, const Matrix& rhs);

void EXPORT_GSL multiply_v_m(Vector& result, const Vector& lhs, const Matrix& rhs); 
void EXPORT_GSL multiply_m_v(Vector& result, const Matrix& rhs, const Vector& lhs);

double EXPORT_GSL dot(const gsl_vector *lhs, const gsl_vector *rhs); 

void EXPORT_GSL matrix_orthogonalize(Matrix& M); 


}


#endif 
