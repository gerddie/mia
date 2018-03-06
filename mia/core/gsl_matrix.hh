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

#ifndef GSLPP_MATRIX_HH
#define GSLPP_MATRIX_HH

#include <cassert>
#include <iterator>
#include <gsl/gsl_matrix.h>
#include <mia/core/gsl_vector.hh>

namespace gsl
{

class EXPORT_GSL matrix_iterator
{
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


       matrix_iterator(const matrix_iterator& other) = default;

       double& operator *()
       {
              assert(m_current);
              return *m_current;
       }

       matrix_iterator& operator ++()
       {
              assert(m_current);
              ++m_current;
              ++m_current_column;

              if (m_current_column == m_matrix->size2) {
                     m_current += m_row_jump;
                     m_current_column = 0;
              }

              return *this;
       }


       matrix_iterator operator ++(int)
       {
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

bool EXPORT_GSL operator == (const matrix_iterator& lhs, const matrix_iterator& rhs);
bool EXPORT_GSL operator != (const matrix_iterator& lhs, const matrix_iterator& rhs);


class EXPORT_GSL const_matrix_iterator
{
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


       const_matrix_iterator(const const_matrix_iterator& other) = default;

       double operator *() const
       {
              assert(m_current);
              return *m_current;
       }

       const_matrix_iterator& operator ++()
       {
              assert(m_current);
              ++m_current;
              ++m_current_column;

              if (m_current_column == m_matrix->size2) {
                     m_current += m_row_jump;
                     m_current_column = 0;
              }

              return *this;
       }

       const_matrix_iterator operator ++(int)
       {
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

bool EXPORT_GSL operator == (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs);
bool EXPORT_GSL operator != (const const_matrix_iterator& lhs, const const_matrix_iterator& rhs);





/**
   This is a wrapper class around the GSL matrix type. It provides
   a compatibility to avoid handling de-alloction manually.
*/
class EXPORT_GSL Matrix
{
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
          move constructor
        */
       Matrix(Matrix&& other);


       /**
          Wrap an existing GSL matrix
          \param m the GSL matrix
        */
       Matrix(gsl_matrix *m);

       /**
          Wrap an existing GSL constant matrix
          \param m the GSL matrix
        */
       Matrix(const gsl_matrix *m);

       /**
          Acquire the transposed matrix.
          \returns  transposed matrix as newly created object
       */
       Matrix transposed() const;


       /**
          Copy operator that executes a deep copy
        */
       Matrix& operator =(const Matrix& other);

       /**
          Move operator that
        */
       Matrix& operator =(Matrix&& other);


       /**
          Reset the matrix with the new dimensions.
          \param rows
          \param columns
          \param clean - set all values to zero
        */
       void reset(size_t rows, size_t columns, bool clean);

       /**
          Reset the matrix with the new dimensions.
          \param rows
          \param columns
          \param init - set all values to \a init
        */
       void reset(size_t rows, size_t columns, double init);

       ~Matrix();


       /**
          \returns the number of rows in the matrix
        */
       size_t rows()const;

       /**
          \returns the number of columns in the matrix
       */
       size_t cols()const;

       /**
          Set a value of the matrix
          \param i row
          \param j column
          \param x value
        */
       void set(size_t i, size_t j, double x);

       /**
          Get a value of the matrix
          \param i row
          \param j column
          \returns the value
        */

       double operator ()(size_t i, size_t j) const;

       /**
          operator to get the underlying gsl_matrix pointer
        */
       operator gsl_matrix *();

       /**
          operator to get the underlying const gsl_matrix pointer
        */
       operator const gsl_matrix *() const;

       /**
          Evaluate the covariance matrix between the columns of this matrix
          \returns the covariance matrix
        */
       Matrix column_covariance() const;

       /**
          Evaluate the covariance matrix between the rows of this matrix
          \returns the covariance matrix
        */
       Matrix row_covariance() const;

       /**
          Iterator over the matrix elements of the matrix, column indices are the
          fastest changing indices.
          \returns the begin of the range
        */
       matrix_iterator begin();

       /**
          Iterator over the matrix elements of the matrix, column indices are the
          fastest changing indices.
          \returns the end of the range
        */
       matrix_iterator end();

       /**
          Read only iterator over the matrix elements of the matrix, column indices are the
          fastest changing indices.
          \returns the begin of the range
        */
       const_matrix_iterator begin() const;

       /**
          Read only iterator over the matrix elements of the matrix, column indices are the
          fastest changing indices.
          \returns the end of the range
        */
       const_matrix_iterator end() const;

       /**
          Set a matrix row
          \param r index of row to be set, must be in [0, this->rows()-1]
          \param row vector containing the new values. It's size must be equal
          to the number of columns of the matrix
        */
       void set_row(int r, const Vector& row);

       /**
          Get a read-write view of a matrix row
          \param r index of the matrix row
          \returns view
       */
       VectorView get_row(int r);

       /**
          Get a read-only view of a matrix row
          \param r index of the matrix row
          \returns constant view
       */
       ConstVectorView get_row(int r) const;

       /**
          Set a matrix column
          \param c index of column to be set,  must be in [0, this->cols()-1]
          \param col vector containing the new values. It's size must be equal
          to the number of rows of the matrix
        */
       void set_column(int c, const Vector& col);

       /**
          Get a read-write view of a matrix column
          \param c index of the matrix column
          \returns view
       */
       VectorView get_column(int c);

       /**
          Get a read-only view of a matrix column
          \param c index of the matrix column
          \returns view
       */
       ConstVectorView get_column(int c) const;

       /**
          Evaluate the dot product between a row of the matrix and a given vector
          \param r index of row, must be in [0, this->rows()-1]
          \param v vector to evaluate the dot product with. The vector's size must be equal to the
          number of matrix columns.
          \returns the dot product of the row vector and the input vector
        */
       double dot_row(int r, const Vector& v) const;

       /**
          Evaluate the dot product between a column  of the matrix and a given vector
          \param c index of column,  must be in [0, this->cols()-1]
       \param col vector to evaluate the dot product with. The vector's size must be equal to the
          number of matrix rows.
          \returns the dot product of the row vector and the input vector
        */

       double dot_column(int c, const Vector& col) const;

       /**
          Print matrix to a ostream
          \param os output stream to write to
        */
       void print(std::ostream& os) const;

       /**
          Element wise matrix subtraction
          \param rhs matrix to subtract
          \returns reference of this matrix
       */
       Matrix& operator -=(const Matrix& rhs)
       {
              gsl_matrix_sub(*this, rhs);
              return *this;
       }

       /**
          Element wise matrix addition
          \param rhs matrix to add
          \returns reference of this matrix
       */
       Matrix& operator +=(const Matrix& rhs)
       {
              gsl_matrix_add(*this, rhs);
              return *this;
       }

       /**
          Scale elements with a factor
          \param rhs scaling factor
          \returns reference of this matrix
       */
       Matrix& operator *=(double rhs)
       {
              gsl_matrix_scale(*this, rhs);
              return *this;
       }

       bool is_valid() const;

       bool is_writable() const;

private:
       gsl_matrix *m_matrix;
       const gsl_matrix *m_const_matrix;
       bool m_owner;
};


Matrix EXPORT_GSL operator * (const Matrix& lhs, const Matrix& rhs);
Matrix EXPORT_GSL operator + (const Matrix& lhs, const Matrix& rhs);
Matrix EXPORT_GSL operator - (const Matrix& lhs, const Matrix& rhs);

inline std::ostream& operator << (std::ostream& os, const Matrix& m)
{
       m.print(os);
       return os;
}

/**
   Evaluate the eigenvalues and eigenvectors of the input matrix
*/
struct EXPORT_GSL CSymmvEvalEvec {
       CSymmvEvalEvec(Matrix m);

       Matrix evec;
       Vector eval;
};

/**
   Evaluate in place: pow(m, -0.5);

*/
void EXPORT_GSL matrix_inv_sqrt(Matrix& m);


} // end namespace

namespace std
{

template <>
class iterator_traits< gsl::const_matrix_iterator >
{
public:
       typedef size_t   difference_type;
       typedef double	 value_type;
       typedef const double *pointer;
       typedef const double&	reference;
       typedef forward_iterator_tag	iterator_category;
};

template <>
class iterator_traits< gsl::matrix_iterator >
{
public:
       typedef size_t   difference_type;
       typedef double	 value_type;
       typedef double *pointer;
       typedef double&	reference;
       typedef forward_iterator_tag	iterator_category;
};

}


#endif
