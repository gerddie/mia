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
#ifndef __mia_3d_matrix_hh
#define __mia_3d_matrix_hh

#include <vector>
#include <memory>

#include <mia/3d/vector.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

template <typename T>
using T3DCVector = T3DVector<std::complex<T>>;

/**
   @ingroup basic
   \brief a simple 3x3 matrix

   This si a simple implementation of a 3x3 matrix that supports the evaluation of certain
   properties and operations with vectors

   \tparam T the data type of the elements of the matrix
 */

template <typename T>
class T3DMatrix: public T3DVector< T3DVector<T>>
{


public:


       T3DMatrix();


       /**
          Create a diagonal matrix
          \param value the value to set the diagonal elements to
          \returns a diagonal matrix with the gibe diagonal
       */
       static T3DMatrix<T> diagonal(T value);

       /**
          Create a diagonal matrix
          \param values the values to set the diagonal elements to a(0,0) = values.x, a(1,1) = values.y, ...
          \returns a diagonal matrix with the gibe diagonal
       */
       static T3DMatrix<T> diagonal(const T3DVector<T>& values);


       /**
          Construct a matrix by copying from a differenty typed matrix
          \tparam I the element type of the original matrix
          \param o the matrix to be copied
        */
       template <typename I>
       T3DMatrix(const T3DMatrix<I>& o);


       /**
          Construct the matrix by giving a 3D vector of 3D vectors
          \remark This is needed to make transparent use of the T3DVector operators
          \param other  the input matrix
       */
       T3DMatrix(const T3DVector< T3DVector<T>>& other);


       /**
          Construct the matrix by giving the rows as 3D vectors
          \param x 1st row
          \param y 2st row
          \param z 3rd row
       */
       T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z );

       /**
          inplace subtract
          \param other
          \returns
        */
       T3DMatrix<T>& operator -= (const T3DMatrix<T>& other);


       /**
          print the matrix to an ostream
          \param os the output stream
       */
       void print( std::ostream& os) const;


       /**
          \returns the transposed of this matrix
        */
       T3DMatrix<T>  transposed()const;


       /**
          \returns the determinat of the matrix
        */
       T get_det()  const;


       /**
          \returns the rank of the matrix
        */
       int get_rank()const;

       /** calculated the eigenvalues of the matrix using the caracteristic polynome, and
           Cardans formula
           \retval result stores the three eigenvalues, interprete dependend on returns

           \returns 1 one real, two complex eigenvalues, real part = result->y, imaginary part = result->z
                    2 three real eigenvalues, at least two are equal
       	     3 three distinct real eigenvalues
       	     4 three real eigenvalues, all equal
       */

       int get_eigenvalues(T3DVector<T>& v)const;

       /** Calculate the eigenvector to a given eigenvalues.
           This function doesn't work for complex valued eigenvectors
           \param i number of eigenvector
           \returns the requested eigenvector
        */
       T3DVector<T> get_real_eigenvector(int i)const;

       /** Calculate the complex eigenvector to a given eigenvalues.
           \param i number of eigenvector
           \returns the requested eigenvector
        */
       T3DCVector<T> get_complex_eigenvector(int i)const;


       /// The unity matrix
       static const T3DMatrix _1;

       /// The zero matrix
       static const T3DMatrix _0;

private:
       void evaluate_ev() const;


       mutable int m_ev_type; // 0 = not valid
       mutable T3DVector<T> m_evalues;
       mutable std::vector<T3DCVector<T>> m_complex_evectors;
       mutable std::vector<int> m_ev_order;
};

template <typename T>
template <typename I>
T3DMatrix<T>::T3DMatrix(const T3DMatrix<I>& o):
       T3DVector<T3DVector<T>>(T3DVector<T>(o.x),
                               T3DVector<T>(o.y),
                               T3DVector<T>(o.z)),
                            m_ev_type(0)
{
}

template <typename T>
T3DVector<T> operator * (const T3DMatrix<T>& m, const T3DVector<T>& x)
{
       return T3DVector<T>(dot(m.x, x), dot(m.y, x), dot(m.z, x));
}

template <typename T>
T3DVector<T> operator * (const T3DVector<T>& x, const T3DMatrix<T>& m )
{
       return T3DVector<T>(m.x.x * x.x + m.y.x * x.y + m.z.x * x.z,
                           m.x.y * x.x + m.y.y * x.y + m.z.y * x.z,
                           m.x.z * x.x + m.y.z * x.y + m.z.z * x.z);
}


template <typename T>
std::ostream& operator << (std::ostream& os, const T3DMatrix<T>& m)
{
       m.print(os);
       return os;
}

template <typename T>
T3DMatrix<T>& T3DMatrix<T>::operator -= (const T3DMatrix<T>& o)
{
       m_ev_type = 0;
       this->x -= o.x;
       this->y -= o.y;
       this->z -= o.z;
       return *this;
}

template <typename T>
T3DMatrix<T> operator * (const T3DMatrix<T>& m, const T3DMatrix<T>& x  )
{
       return T3DMatrix<T>(T3DVector<T>(m.x.x * x.x.x + m.x.y * x.y.x + m.x.z * x.z.x,
                                        m.x.x * x.x.y + m.x.y * x.y.y + m.x.z * x.z.y,
                                        m.x.x * x.x.z + m.x.y * x.y.z + m.x.z * x.z.z),
                           T3DVector<T>(m.y.x * x.x.x + m.y.y * x.y.x + m.y.z * x.z.x,
                                        m.y.x * x.x.y + m.y.y * x.y.y + m.y.z * x.z.y,
                                        m.y.x * x.x.z + m.y.y * x.y.z + m.y.z * x.z.z),
                           T3DVector<T>(m.z.x * x.x.x + m.z.y * x.y.x + m.z.z * x.z.x,
                                        m.z.x * x.x.y + m.z.y * x.y.y + m.z.z * x.z.y,
                                        m.z.x * x.x.z + m.z.y * x.y.z + m.z.z * x.z.z));
}

/// a simple 3x3 matrix with single precision floating point values
typedef T3DMatrix<float> C3DFMatrix;

/// a simple 3x3 matrix with double precision floating point values
typedef T3DMatrix<double> C3DDMatrix;


template <typename T>
const T3DMatrix<T> T3DMatrix<T>::_1(T3DVector< T >(1, 0, 0),
                                    T3DVector< T >(0, 1, 0),
                                    T3DVector< T >(0, 0, 1));

template <typename T>
const T3DMatrix<T> T3DMatrix<T>::_0 = T3DMatrix<T>();

extern template class EXPORT_3D T3DMatrix<float>;
extern template class EXPORT_3D T3DMatrix<double>;


NS_MIA_END

#endif
