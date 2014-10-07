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

#ifndef GSLPP_MATRIX_HH
#define GSLPP_MATRIX_HH

#include <cassert>
#include <iterator>
#include <gsl/gsl_matrix.h>
#include <gsl++/vector.hh>
#include <gsl++/gsldefines.hh> 

namespace gsl {

class EXPORT_GSL matrix_iterator {
        friend class const_matrix_iterator; 
public: 
	matrix_iterator(gsl_matrix *m, bool begin):
		m_matrix(m), 
		m_current(begin ? m->data : m->data +  m->size1 * m->tda), 
		m_current_column(0), 
		m_row_jump(m->tda - m->size2)
		{
		}
		
	matrix_iterator():
		m_matrix(nullptr), 
		m_current(nullptr), 
		m_current_column(0)
		{
		}
		
		
	matrix_iterator(const matrix_iterator& other):
		m_matrix(other.m_matrix), 
		m_current(other.m_current), 
		m_current_column(other.m_current_column)
		{
		}

	double& operator *(){
		assert(m_current); 
		return *m_current; 
	}
	
	matrix_iterator& operator ++() {
		++m_current; 
		++m_current_column; 
		if(m_current_column == m_matrix->size2) {
			m_current += m_row_jump; 
			m_current_column = 0; 
		}
		return *this; 
	}
	
	
	matrix_iterator operator ++(int) {
		matrix_iterator result(*this); 
		++(*this); 
		return result; 
	}
	
	friend bool operator == (const matrix_iterator& lhs, const matrix_iterator& rhs); 
	friend bool operator != (const matrix_iterator& lhs, const matrix_iterator& rhs); 
private: 
	gsl_matrix *m_matrix; 
	double *m_current; 
	size_t m_current_column; 
	size_t m_row_jump; 
}; 

bool operator == (const matrix_iterator& lhs, const matrix_iterator& rhs); 
bool operator != (const matrix_iterator& lhs, const matrix_iterator& rhs); 


class EXPORT_GSL const_matrix_iterator {
public: 
	const_matrix_iterator(const gsl_matrix *m, bool begin):
		m_matrix(m), 
		m_current(begin ? m->data : m->data +  m->size1 * m->tda), 
		m_current_column(0), 
		m_row_jump(m->tda - m->size2)
		{
		}

	const_matrix_iterator(const matrix_iterator& other):
		m_matrix(other.m_matrix), 
		m_current(other.m_current), 
		m_current_column(other.m_current_column), 
		m_row_jump(other.m_row_jump)
		{
		}
		
	const_matrix_iterator():
		m_matrix(nullptr), 
		m_current(nullptr), 
		m_current_column(0)
		{
		}
		
		
	const_matrix_iterator(const const_matrix_iterator& other):
		m_matrix(other.m_matrix), 
		m_current(other.m_current), 
		m_current_column(other.m_current_column), 
		m_row_jump(other.m_row_jump)
		{
		}

	double operator *() const{
		assert(m_current); 
		return *m_current; 
	}
	
	const_matrix_iterator& operator ++() {
		++m_current; 
		++m_current_column; 
		if(m_current_column == m_matrix->size2) {
			m_current += m_row_jump; 
			m_current_column = 0; 
		}
		return *this; 
	}
			
	const_matrix_iterator operator ++(int) {
		const_matrix_iterator result(*this); 
		++(*this); 
		return result; 
	}

	friend bool operator == (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs); 
	friend bool operator != (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs); 
private: 
	const gsl_matrix *m_matrix; 
	const double *m_current; 
	size_t m_current_column; 
	size_t m_row_jump; 
}; 

bool operator == (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs); 
bool operator != (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs); 


/**
   This is a wrapper class around the GSL matrix type. It provides 
   a compatibility to avoid handling de-alloction manually.
*/
class EXPORT_GSL Matrix {
public: 
	typedef matrix_iterator iterator; 
	typedef const_matrix_iterator const_iterator; 

	Matrix(); 

	/**
	   Create a matrix of size rows \f$\times\f$ columns, 
	   \param rows
	   \param columns 
	   \param clean allocate zeroing out all elements 
	 */
	Matrix(size_t rows, size_t columns, bool clean); 

	/**
	   Create a matrix of size rows \f$\times\f$ columns, 
	   \param rows
	   \param columns 
	   \param init set the matrix elements to this value 
	 */
	Matrix(size_t rows, size_t columns, double init); 

	/**
	   Create a matrix of size rows \f$\times\f$ columns and initialize it 
	   with the given data 
	   \param rows
	   \param columns 
	   \param init the input data in row major format 
	*/
	Matrix(size_t rows, size_t columns, const double *init);
	
	/**
	   Copy constructor that executes a deep copy 
	 */
	Matrix(const Matrix& other); 
	
	/**
	   Wrap an existing GSL matrix 
	   \param m the GSL matrix 
	 */
	Matrix(gsl_matrix* m); 

	Matrix(const gsl_matrix* m); 
	/**
	   Acquire the transposed matrix. 
	   \returns  transposed matrix as newly created object
	*/
	Matrix transposed() const; 


	/**
	   Copy operator that executes a deep copy 
	 */
	Matrix& operator =(const Matrix& other); 

	void reset(size_t rows, size_t columns, bool clean); 
	void reset(size_t rows, size_t columns, double init); 

	~Matrix(); 

	size_t rows()const; 
	size_t cols()const; 
	
	void set(size_t i, size_t j, double x); 
	double operator ()(size_t i, size_t j) const; 
	
	operator gsl_matrix *(); 
	operator const gsl_matrix *() const; 

	Matrix column_covariance() const; 
	Matrix row_covariance() const; 

	matrix_iterator begin(); 
	matrix_iterator end(); 

	const_matrix_iterator begin() const; 
	const_matrix_iterator end() const; 

	void set_row(int r, const DoubleVector& row); 
	void set_column(int r, const DoubleVector& row); 

	double dot_row(int r, const DoubleVector& row) const; 
	double dot_column(int c, const DoubleVector& col) const; 
	
private: 
	gsl_matrix *m_matrix; 
	const gsl_matrix *m_const_matrix; 
	bool m_owner; 
}; 


} // end namespace 

namespace std {

template <>
class iterator_traits< gsl::const_matrix_iterator >  {
public: 
	typedef size_t   difference_type; 
	typedef double	 value_type; 
	typedef const double* pointer; 
	typedef const double&	reference; 
	typedef forward_iterator_tag	iterator_category; 
}; 

template <>
class iterator_traits< gsl::matrix_iterator > {
public: 
	typedef size_t   difference_type; 
	typedef double	 value_type; 
	typedef double* pointer; 
	typedef double&	reference; 
	typedef forward_iterator_tag	iterator_category; 
}; 

}


#endif
