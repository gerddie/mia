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

#include <gsl++/matrix.hh>

namespace gsl {

Matrix::Matrix():m_matrix(NULL)
{
}

Matrix::Matrix(size_t rows, size_t columns, bool clean):
	m_matrix(NULL)
{
	m_matrix = clean ? 
		gsl_matrix_calloc(rows, columns):
		gsl_matrix_alloc(rows, columns); 
}

Matrix::Matrix(size_t rows, size_t columns, const double *init):
	m_matrix(NULL)
{
	assert(init); 
	m_matrix = gsl_matrix_alloc(rows, columns); 
	memcpy(gsl_matrix_ptr (m_matrix, 0, 0), init, rows * columns * sizeof(double)); 
}

Matrix::Matrix(const Matrix& other)
{
	m_matrix = gsl_matrix_alloc(other.rows(), other.cols()); 
	gsl_matrix_memcpy (m_matrix, other.m_matrix); 
}

Matrix& Matrix::operator =(const Matrix& other)
{
	if (this == &other) 
		return *this; 

	if (m_matrix && rows() == other.rows() && cols() == other.cols()) {
		gsl_matrix_memcpy (m_matrix, other.m_matrix);
		return *this; 
	}
	gsl_matrix *help = gsl_matrix_alloc(other.rows(), other.cols()); 
	gsl_matrix_memcpy (help, other.m_matrix); 
	if (m_matrix) 
		gsl_matrix_free(m_matrix);
	m_matrix = help; 
	return *this; 
}

Matrix::~Matrix()
{
	if (m_matrix) 
		gsl_matrix_free(m_matrix);
}

size_t Matrix::rows()const
{
	assert(m_matrix); 
	return m_matrix->size1; 
}

size_t Matrix::cols()const
{
	assert(m_matrix); 
	return m_matrix->size2; 
}


void Matrix::set(size_t i, size_t j, double x)
{
	gsl_matrix_set(m_matrix, i,j,x); 
}

double Matrix::operator ()(size_t i, size_t j) const
{
	return gsl_matrix_get(m_matrix, i,j);
}

Matrix::operator gsl_matrix * ()
{
	return m_matrix; 
}

Matrix::operator const gsl_matrix *() const
{
	return m_matrix; 
}

Matrix Matrix::transposed() const
{
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
		auto tmp = gsl_matrix_row (m_matrix, r);
		auto mean = gsl_stats_mean (tmp.vector.data, tmp.vector.stride, n); 
		for (int c = 0; c < n; ++c)  {
			help.set(r, c, gsl_matrix_get(m_matrix, r, c) - mean); 
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
		auto tmp = gsl_matrix_column (m_matrix, i);
		auto mean = gsl_stats_mean (tmp.vector.data, tmp.vector.stride, n); 
		for (int r = 0; r < n; ++r)  {
			help.set(r,i, gsl_matrix_get(m_matrix, r, i) - mean); 
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


}
