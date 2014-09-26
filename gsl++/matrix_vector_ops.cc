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


#include <gsl++/matrix_vector_ops.hh>

#include <gsl/gsl_blas.h>
#include <cassert>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>



namespace gsl {
using namespace tbb;

struct MultVectMatrix {
        typedef   DoubleVector::vector_pointer_type pvector; 
        MultVectMatrix(gsl_vector& result, const gsl_vector& lhs, const Matrix& rhs); 
        
        void operator () (const blocked_range<int>& range) const; 
private:
        gsl_vector& m_result; 
        const gsl_vector& m_lhs; 
        const Matrix& m_rhs; 
}; 

MultVectMatrix::MultVectMatrix(gsl_vector& result, const gsl_vector& lhs, const Matrix& rhs):
        m_result(result), 
        m_lhs(lhs), 
        m_rhs(rhs)
{
}

void MultVectMatrix::operator () (const blocked_range<int>& range) const
{
        for (int c = range.begin(); c != range.end(); ++c) {
                auto rhs_column = gsl_matrix_const_column(m_rhs, c); 
                const double val = cblas_ddot (m_rhs.rows(), m_lhs.data, m_lhs.stride, 
                                         rhs_column.vector.data, rhs_column.vector.stride); 
                gsl_vector_set(&m_result, c, val); 
        }
}

void multiply_m_m(Matrix& result, const Matrix& lhs, const Matrix& rhs)
{
        assert(result.rows() == lhs.rows()); 
        assert(result.cols() == rhs.cols()); 
        assert(lhs.cols() == rhs.rows());

        for (unsigned r = 0; r < result.rows(); ++r) {
                gsl_vector_view out_row = gsl_matrix_row(result, r); 
                gsl_vector_const_view lhs_row = gsl_matrix_const_row(lhs, r); 
                MultVectMatrix op(out_row.vector, lhs_row.vector, rhs); 
                parallel_for(blocked_range<int>( 0, result.cols()), op);
        }
}

void multiply_v_m(DoubleVector& result, const DoubleVector& lhs, const Matrix& rhs)
{
        assert(result.size() == rhs.cols()); 
        assert(lhs.size() == rhs.rows());

        MultVectMatrix op(*result, *lhs, rhs);
        parallel_for(blocked_range<int>( 0, result.size()), op);
}

void multiply_m_v(DoubleVector& result, const Matrix& lhs, const DoubleVector& rhs)
{
        assert(result.size() == lhs.rows()); 
        assert(rhs.size() == lhs.cols());
        
        const gsl_vector vrhs = *rhs;  
        auto mult = [&result, &lhs, &vrhs](const blocked_range<int>& range) -> void {
                for (int r = range.begin(); r != range.end(); ++r) {
                        auto lhs_row = gsl_matrix_const_row(lhs, r); 
                        const double val = cblas_ddot (result.size(), lhs_row.vector.data, lhs_row.vector.stride, 
                                                       vrhs.data, vrhs.stride);
                        result[r] = val; 
                }
        }; 
        parallel_for(blocked_range<int>( 0, result.size()), mult);
        
}

}
