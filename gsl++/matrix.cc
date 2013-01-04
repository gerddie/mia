/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cassert>
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

}


