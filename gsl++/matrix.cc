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

#include <cassert>
#include <cstring>
#include <iostream>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>

#include <gsl++/matrix.hh>
#include <gsl++/matrix_vector_ops.hh>
#include <algorithm>

namespace gsl {

using std::swap; 
using std::fill; 
using std::ostream_iterator; 

Matrix::Matrix():m_matrix(nullptr), m_const_matrix(nullptr), m_owner(false)
{
}

Matrix::Matrix(size_t rows, size_t columns, bool clean):
	m_matrix(NULL), m_owner(true)
{
	m_matrix = clean ? 
		gsl_matrix_calloc(rows, columns):
		gsl_matrix_alloc(rows, columns); 
	m_const_matrix = m_matrix; 
}

Matrix::Matrix(size_t rows, size_t columns, double init):
	m_matrix(NULL), m_owner(true)
{
	m_matrix = gsl_matrix_alloc(rows, columns); 
	auto p = gsl_matrix_ptr (m_matrix, 0, 0);
	fill(p, p + rows * columns, init); 
	m_const_matrix = m_matrix; 
}

Matrix::Matrix(size_t rows, size_t columns, const double *init):
	m_matrix(NULL), m_owner(true)
{
	assert(init); 
	m_matrix = gsl_matrix_alloc(rows, columns); 
	memcpy(gsl_matrix_ptr (m_matrix, 0, 0), init, rows * columns * sizeof(double)); 
	m_const_matrix = m_matrix; 
}

Matrix::Matrix(const Matrix& other):
	m_owner(true)
{
	m_matrix = gsl_matrix_alloc(other.rows(), other.cols()); 
	gsl_matrix_memcpy (m_matrix, other.m_matrix);
	m_const_matrix = m_matrix; 
}

Matrix::Matrix(gsl_matrix* m):
	m_matrix(m), 
	m_const_matrix(m), 
	m_owner(false)
{
}

Matrix::Matrix(const gsl_matrix* m):
	m_matrix(nullptr), 
	m_const_matrix(m), 
	m_owner(false)
{
}

Matrix& Matrix::operator =(const Matrix& other)
{
	if (this == &other) 
		return *this; 

	if (m_matrix && rows() == other.rows() && cols() == other.cols()) {
		gsl_matrix_memcpy (m_matrix, other.m_const_matrix);
		return *this; 
	}
	gsl_matrix *help = gsl_matrix_alloc(other.rows(), other.cols()); 
	gsl_matrix_memcpy (help, other.m_matrix); 
	swap(m_matrix, help); 
	if (help && m_owner) 
		gsl_matrix_free(help);
	
	m_const_matrix = m_matrix; 
	m_owner = true; 
	return *this; 
}

void Matrix::reset(size_t rows, size_t columns, bool clean) 
{
	gsl_matrix *help = clean ? 
		gsl_matrix_calloc(rows, columns):
		gsl_matrix_alloc(rows, columns); 
	swap(help, m_matrix); 
	if (help && m_owner) 
		gsl_matrix_free(help);
	m_const_matrix = m_matrix; 
}

void Matrix::reset(size_t rows, size_t columns, double init) 
{
	gsl_matrix *help = gsl_matrix_alloc(rows, columns); 
	auto p = gsl_matrix_ptr (help, 0, 0);
	fill(p, p + rows * columns, init); 
	
	swap(help, m_matrix); 
	if (help && m_owner) 
		gsl_matrix_free(help);
	m_const_matrix = m_matrix; 
}

Matrix::~Matrix()
{
	if (m_matrix && m_owner) 
		gsl_matrix_free(m_matrix);
}

size_t Matrix::rows()const
{
	assert(m_const_matrix); 
	return m_const_matrix->size1; 
}

size_t Matrix::cols()const
{
	assert(m_const_matrix); 
	return m_const_matrix->size2; 
}

void Matrix::set_row(int r, const Vector& row)
{
	assert(row.size() == cols()); 
	auto mrow = gsl_matrix_row(m_matrix, r); 
	gsl_vector_memcpy(&mrow.vector, row); 
}

VectorView Matrix::get_row(int r)
{
	return VectorView(gsl_matrix_row(m_matrix, r)); 
}

ConstVectorView Matrix::get_row(int r) const 
{
	return ConstVectorView(gsl_matrix_const_row(m_matrix, r)); 
}

void Matrix::set_column(int c, const Vector& col)
{
	assert(col.size() == rows()); 
	auto mcol = gsl_matrix_column(m_matrix, c); 
	gsl_vector_memcpy(&mcol.vector, col); 
}

VectorView Matrix::get_column(int c)
{
	return VectorView(gsl_matrix_column(m_matrix, c)); 
}

ConstVectorView Matrix::get_column(int c) const
{
	return ConstVectorView(gsl_matrix_const_column(m_matrix, c)); 
}



double Matrix::dot_row(int r, const Vector& row) const 
{
	auto mrow = gsl_matrix_const_row(m_const_matrix, r); 
	return dot(row, &mrow.vector); 
}

double Matrix::dot_column(int c, const Vector& col) const 
{
	auto mcol = gsl_matrix_const_column(m_const_matrix, c); 
	return dot(col, &mcol.vector); 
}

void Matrix::print(std::ostream& os) const
{
	os << "["; 
	if (!m_matrix) {
		if (!m_const_matrix) {
			os << "(null)]\n"; 
			return; 
		}
			
		os << "(const)"; 
	}
	os << "\n"; 
	
	for (unsigned r = 0; r < rows(); ++r) {
		auto mrow = get_row(r); 
		os << "  "; 
		copy(mrow.begin(), mrow.end(), ostream_iterator<double>(os, ", ")); 
		os << "\n"; 
	}
	os << "]"; 
}

void Matrix::set(size_t i, size_t j, double x)
{
	assert(m_matrix); 
	gsl_matrix_set(m_matrix, i,j,x); 
}

double Matrix::operator ()(size_t i, size_t j) const
{
	return gsl_matrix_get(m_const_matrix, i,j);
}

Matrix::operator gsl_matrix * ()
{
	return m_matrix; 
}

Matrix::operator const gsl_matrix *() const
{
	return m_const_matrix; 
}


matrix_iterator Matrix::begin()
{
	assert(m_matrix); 
	return matrix_iterator(m_matrix, true); 
}

matrix_iterator Matrix::end()
{
	assert(m_matrix); 
	return matrix_iterator(m_matrix, false);
}


const_matrix_iterator Matrix::begin() const
{
	assert(m_const_matrix); 
	return const_matrix_iterator(m_const_matrix, true); 
}

const_matrix_iterator Matrix::end() const
{
	assert(m_const_matrix); 
	return const_matrix_iterator(m_const_matrix, false); 
}

Matrix Matrix::transposed() const
{
	assert(m_matrix); 
	Matrix result(cols(), rows(), false); 
	gsl_matrix_transpose_memcpy (result.m_matrix, m_matrix); 
	return result; 
}

Matrix Matrix::row_covariance() const 
{
	int d = rows(); 
	int n = cols(); 
	Matrix cov(d,d, true); 
	
	Matrix help(d, n, false); 

	// remove mean 
	for (int r = 0; r < d; ++r)  {
		auto tmp = gsl_matrix_const_row (m_const_matrix, r);
		auto mean = gsl_stats_mean (tmp.vector.data, tmp.vector.stride, n); 
		for (int c = 0; c < n; ++c)  {
			help.set(r, c, gsl_matrix_get(m_const_matrix, r, c) - mean); 
		}
	}

	for (int i = 0; i < d; i++) {
		for (int j = 0; j <= i; j++) {
			
			auto a = gsl_matrix_row (help, i);
			auto b = gsl_matrix_row (help, j);
			double c = gsl_stats_covariance(a.vector.data, a.vector.stride,
							b.vector.data, b.vector.stride, n);
			cov.set(j, i, c); 
			cov.set(i, j, c); 
		}
	}
	return cov; 
}

Matrix Matrix::column_covariance() const
{
	int n = rows(); 
	int d = cols(); 
	Matrix cov(d,d, true); 
	
	Matrix help(n, d, false); 

	// remove mean 
	for (int i = 0; i < d; ++i)  {
		auto tmp = gsl_matrix_const_column (m_const_matrix, i);
		auto mean = gsl_stats_mean (tmp.vector.data, tmp.vector.stride, n); 
		for (int r = 0; r < n; ++r)  {
			help.set(r,i, gsl_matrix_get(m_const_matrix, r, i) - mean); 
		}
	}

	for (int i = 0; i < d; i++) {
		for (int j = 0; j <= i; j++) {
			
			auto a = gsl_matrix_column (help, i);
			auto b = gsl_matrix_column (help, j);
			double c = gsl_stats_covariance(a.vector.data, a.vector.stride,
							b.vector.data, b.vector.stride, n);
			cov.set(j, i, c); 
			cov.set(i, j, c); 
		}
	}
	return cov; 
	
}


Matrix operator - (const Matrix& lhs, const Matrix& rhs)
{
	Matrix result(lhs); 
	result -= rhs; 
	return result; 
}

Matrix operator + (const Matrix& lhs, const Matrix& rhs)
{
	Matrix result(lhs); 
	result += rhs; 
	return result; 
}

Matrix operator * (const Matrix& lhs, const Matrix& rhs)
{
	Matrix result(lhs.rows(), rhs.cols(), false); 
	multiply_m_m(result, lhs, rhs);
	return result; 
}

CSymmvEvalEvec::CSymmvEvalEvec(Matrix m):
	evec(m.rows(), m.cols(), false), 
	eval(m.rows(), false)
{
	assert(m.cols() == m.rows()); 
	
	gsl_eigen_symmv_workspace *ws = gsl_eigen_symmv_alloc (m.rows()); 
	gsl_eigen_symmv (m, eval, evec, ws); 
	gsl_eigen_symmv_free (ws); 
}

void matrix_inv_sqrt(Matrix& m )
{
	Matrix mTm(m.cols(), m.cols(), false); 
	multiply_mT_m(mTm, m, m); 

	CSymmvEvalEvec see(mTm);
	
	for (unsigned r = 0; r < see.evec.cols(); ++r) {
		auto wmr = see.evec.get_column(r); 
		const double f = see.eval[r] > 0 ? 1.0/ sqrt(sqrt(see.eval[r])) : 0.0; 
		std::transform(wmr.begin(), wmr.end(), wmr.begin(), [f](double x) {return f*x;}); 
	}
	multiply_m_mT(mTm, see.evec, see.evec); 
	m = m * mTm; 

}


bool operator == (const matrix_iterator& lhs, const matrix_iterator& rhs)
{
	assert(lhs.m_matrix == rhs.m_matrix); 
	return lhs.m_current == rhs.m_current; 
}

bool operator != (const matrix_iterator& lhs, const matrix_iterator& rhs)
{
	assert(lhs.m_matrix == rhs.m_matrix); 
	return lhs.m_current != rhs.m_current; 
}

bool operator == (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs)
{
	assert(lhs.m_matrix == rhs.m_matrix); 
	return lhs.m_current == rhs.m_current; 
}

bool operator != (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs)
{
	assert(lhs.m_matrix == rhs.m_matrix); 
	return lhs.m_current != rhs.m_current; 
}


}
