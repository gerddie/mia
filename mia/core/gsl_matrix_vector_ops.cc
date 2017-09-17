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


#include <mia/core/gsl_matrix_vector_ops.hh>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <cassert>
#include <mia/core/parallel.hh>

namespace gsl {
using mia::C1DParallelRange;
using mia::pfor;

struct MultVectMatrix {
	typedef   Vector::vector_pointer_type pvector; 
	MultVectMatrix(gsl_vector& result, const gsl_vector& lhs, const Matrix& rhs); 
        
        void operator () (const C1DParallelRange& range) const;
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

void MultVectMatrix::operator () (const C1DParallelRange& range) const
{
	double val = 0.0; 
        for (int c = range.begin(); c != range.end(); ++c) {
                auto rhs_column = m_rhs.get_column(c); 
                gsl_blas_ddot(&m_lhs, rhs_column, &val); 
                gsl_vector_set(&m_result, c, val); 
        }
}


struct MultVectMatrixT {
        typedef   Vector::vector_pointer_type pvector; 
        MultVectMatrixT(gsl_vector& result, const gsl_vector& lhs, const Matrix& rhs); 
        
        void operator () (const C1DParallelRange& range) const;
private:
        gsl_vector& m_result; 
        const gsl_vector& m_lhs; 
        const Matrix& m_rhs; 
}; 

MultVectMatrixT::MultVectMatrixT(gsl_vector& result, const gsl_vector& lhs, const Matrix& rhs):
        m_result(result), 
        m_lhs(lhs), 
        m_rhs(rhs)
{
}

void MultVectMatrixT::operator () (const C1DParallelRange& range) const
{
        for (int c = range.begin(); c != range.end(); ++c) {
                auto rhs_row = gsl_matrix_const_row(m_rhs, c); 
                const double val = cblas_ddot (m_rhs.cols(), m_lhs.data, m_lhs.stride, 
                                         rhs_row.vector.data, rhs_row.vector.stride); 
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
                pfor(C1DParallelRange( 0, result.cols()), op);
        }
}

void multiply_mT_m(Matrix& result, const Matrix& lhs, const Matrix& rhs)
{
        assert(lhs.rows() == rhs.rows());
	if (result.rows() != lhs.cols() || 
	    result.cols() != rhs.cols()) 
		result.reset(lhs.cols(), rhs.cols(), false); 

        for (unsigned r = 0; r < result.rows(); ++r) {
                gsl_vector_view out_row = gsl_matrix_row(result, r); 
                gsl_vector_const_view lhs_col = gsl_matrix_const_column(lhs, r); 
                MultVectMatrix op(out_row.vector, lhs_col.vector, rhs);
                pfor(C1DParallelRange( 0, result.cols()), op);
        }

}

void multiply_m_mT(Matrix& result, const Matrix& lhs, const Matrix& rhs)
{
        assert(lhs.cols() == rhs.cols());

	if (result.rows() != lhs.rows() || 
	    result.cols() != rhs.rows()) 
		result.reset(lhs.rows(), rhs.rows(), false); 


	for (unsigned r = 0; r < result.rows(); ++r) {
                gsl_vector_view out_row = gsl_matrix_row(result, r); 
                gsl_vector_const_view lhs_row = gsl_matrix_const_row(lhs, r); 
                MultVectMatrixT op(out_row.vector, lhs_row.vector, rhs);
                pfor(C1DParallelRange( 0, result.cols()), op);
        }
}


void multiply_v_m(Vector& result, const Vector& lhs, const Matrix& rhs)
{
        assert(result.size() == rhs.cols()); 
        assert(lhs.size() == rhs.rows());

        MultVectMatrix op(*result, *lhs, rhs);
        pfor(C1DParallelRange( 0, result.size()), op);
}

void multiply_m_v(Vector& result, const Matrix& lhs, const Vector& rhs)
{
        assert(result.size() == lhs.rows()); 
        assert(rhs.size() == lhs.cols());
        
        auto mult = [&result, &lhs, &rhs](const C1DParallelRange& range) -> void {
                for (int r = range.begin(); r != range.end(); ++r) {
                        auto lhs_row = gsl_matrix_const_row(lhs, r); 
                        const double val = cblas_ddot (lhs_row.vector.size, lhs_row.vector.data, lhs_row.vector.stride, 
                                                       rhs->data, rhs->stride);
                        result[r] = val; 
                }
        }; 
        pfor(C1DParallelRange( 0, result.size()), mult);
        
}

double dot(const gsl_vector *lhs, const gsl_vector *rhs)
{
        assert(rhs); 
        assert(lhs); 
        assert(rhs->size == lhs->size);
        
        return cblas_ddot (rhs->size, lhs->data, lhs->stride, 
                           rhs->data, rhs->stride);

}

void matrix_orthogonalize(Matrix& M)
{
	Matrix U(M); 
	Matrix V(M.cols(), M.cols(), true); 
	Vector D(M.cols(), true); 
	Vector work_vector(M.cols(), false); 
	
	gsl_linalg_SV_decomp (U, V, D, work_vector); 
	
	multiply_m_mT(M, U, V); 
}

}
