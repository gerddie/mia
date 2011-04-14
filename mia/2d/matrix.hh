/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __mia_2d_matrix_hh
#define __mia_2d_matrix_hh

#include <mia/2d/2DVector.hh>

NS_MIA_BEGIN

/**
   Simple implementation of a 2D Matrix to store 2nd order derivatives. 
 */

template <typename T> 
struct T2DMatrix: public T2DVector< T2DVector<T> > {
	
	T2DMatrix(); 
	T2DMatrix(const T2DMatrix<T>& o); 
	T2DMatrix(const T2DVector< T2DVector<T> >& o); 
	T2DMatrix(const T2DVector< T >& x, const T2DVector< T >& y ); 
	
	T2DMatrix& operator = (const T2DMatrix<T>& o);
	
	T2DMatrix& operator -= (const T2DMatrix<T>& o);

}; 

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
