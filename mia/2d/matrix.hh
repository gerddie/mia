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

#ifndef __mia_2d_matrix_hh
#define __mia_2d_matrix_hh

#include <mia/2d/vector.hh>

NS_MIA_BEGIN

/**
   @ingroup misc 
   \brief A simple 2x2 matrix 
   
   Simple implementation of a 2D Matrix to store 2nd order derivatives. 
 */

template <typename T> 
struct T2DMatrix: public T2DVector< T2DVector<T> > {
	
	T2DMatrix(); 

	/// copy constructor 
	T2DMatrix(const T2DMatrix<T>& o); 

	/// construct the matrix from a 2D vector of 2D vectors 
	T2DMatrix(const T2DVector< T2DVector<T> >& o); 
	
	/** construct  the matrix from two 2D vectors 
	    \param x 1st row 
	    \param y 2nd row 
	*/
	T2DMatrix(const T2DVector< T >& x, const T2DVector< T >& y ); 
	
	/// assignment operator 
	T2DMatrix& operator = (const T2DMatrix<T>& o);
	
	/// element wise subtract operator 
	T2DMatrix& operator -= (const T2DMatrix<T>& o);

	/**
	   \returns the transposed of this matrix 
	 */
	T2DMatrix<T>  transposed()const; 


	static const T2DMatrix _1; 
	static const T2DMatrix _0; 

}; 

template <typename T> 
const T2DMatrix<T> T2DMatrix<T>::_1(T2DVector< T >(1,0), 
				    T2DVector< T >(0,1));

template <typename T> 
const T2DMatrix<T> T2DMatrix<T>::_0;


template <typename T> 
T2DMatrix<T>::T2DMatrix():
	T2DVector<T2DVector<T> >(T2DVector<T>(), T2DVector<T>())
{
}

template <typename T> 
T2DMatrix<T>::T2DMatrix(const T2DMatrix<T>& o):
	T2DVector<T2DVector<T> >(o.x, o.y)
{
}

template <typename T> 
T2DMatrix<T>::T2DMatrix(const T2DVector< T2DVector<T> >& o):
	T2DVector<T2DVector<T> >(o.x, o.y)
{
}

template <typename T> 
T2DMatrix<T>::T2DMatrix(const T2DVector< T >& x, const T2DVector< T >& y ):
	T2DVector<T2DVector<T> >(x, y)
{
}


template <typename T> 
T2DMatrix<T>& T2DMatrix<T>::operator = (const T2DMatrix<T>& o)
{
	this->x = o.x; 
	this->y = o.y; 
	return *this; 
}

template <typename T> 
T2DMatrix<T>& T2DMatrix<T>::operator -= (const T2DMatrix<T>& o)
{
	this->x -= o.x; 
	this->y -= o.y; 
	return *this; 
}

template <typename T> 
T2DMatrix<T>  T2DMatrix<T>::transposed()const
{
	return T2DMatrix<T>(T2DVector< T >(this->x.x, this->y.x), 
			    T2DVector< T >(this->y.y, this->y.y)); 
}

/**
   Left hand side product of a matrix m and a vector x 
   \param x
   \param m
   \returns x * m 
 */

template <typename T> 
T2DVector<T> operator * (const T2DVector<T>& x, const T2DMatrix<T>& m)
{
	return T2DVector<T>(dot(m.x, x), dot(m.y, x)); 
}

/**
   right hand side product of a matrix m and a vector x 
   \param m
   \param x
   \returns m * x
 */

template <typename T> 
T2DVector<T> operator * (const T2DMatrix<T>& m, const T2DVector<T>& x  )
{
	return T2DVector<T>(m.x.x * x.x + m.y.x * x.y, 
			    m.x.y * x.x + m.y.y * x.y);
}

/// a simple 2x2 matrix 
typedef T2DMatrix<float> C2DFMatrix; 

NS_MIA_END

#endif
