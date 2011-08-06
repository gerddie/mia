/* -*- mia-c++  -*-
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

#ifndef __mia_3d_matrix_hh
#define __mia_3d_matrix_hh

#include <mia/3d/3DVector.hh>

NS_MIA_BEGIN

template <typename T> 
struct T3DMatrix: public T3DVector< T3DVector<T> > {
	
	T3DMatrix(); 
	T3DMatrix(const T3DMatrix<T>& o); 
	T3DMatrix(const T3DVector< T3DVector<T> >& o); 
	T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z ); 
	
	T3DMatrix& operator = (const T3DMatrix<T>& o);
	
	T3DMatrix& operator -= (const T3DMatrix<T>& o);
	
	void print( std::ostream& os) const; 

	static const T3DMatrix _1; 

}; 

template <typename T> 
const T3DMatrix<T> T3DMatrix<T>::_1(T3DVector< T >(1,0,0), 
				 T3DVector< T >(0,1,0),
				 T3DVector< T >(0,0,1));

template <typename T> 
T3DMatrix<T>::T3DMatrix():
	T3DVector<T3DVector<T> >(T3DVector<T>(), T3DVector<T>(), T3DVector<T>())
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DMatrix<T>& o):
	T3DVector<T3DVector<T> >(o.x, o.y, o.z)
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T3DVector<T> >& o):
	T3DVector<T3DVector<T> >(o.x, o.y, o.z)
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z ):
	T3DVector<T3DVector<T> >(x, y, z)
{
}
template <typename T> 
void T3DMatrix<T>::print( std::ostream& os) const
{
	os << "<" << this->x << ", " << this->y << ", " << this->z << " >"; 
}

template <typename T> 
std::ostream& operator << (std::ostream& os, const T3DMatrix<T>& m) 
{
	m.print(os); 
	return os; 
}

template <typename T> 
T3DMatrix<T>& T3DMatrix<T>::operator = (const T3DMatrix<T>& o)
{
	this->x = o.x; 
	this->y = o.y; 
	this->z = o.z; 
	return *this; 
}

template <typename T> 
T3DMatrix<T>& T3DMatrix<T>::operator -= (const T3DMatrix<T>& o)
{
	this->x -= o.x; 
	this->y -= o.y; 
	this->z -= o.z; 
	return *this; 
}

template <typename T> 
T3DVector<T> operator * (const T3DVector<T>& x, const T3DMatrix<T>& m)
{
	return T3DVector<T>(dot(m.x, x), dot(m.y, x), dot(m.z, x)); 
}

template <typename T> 
T3DVector<T> operator * (const T3DMatrix<T>& m, const T3DVector<T>& x  )
{
	return T3DVector<T>(m.x.x * x.x + m.y.x * x.y + m.z.x * x.z, 
			    m.x.y * x.x + m.y.y * x.y + m.z.y * x.z,
			    m.x.z * x.x + m.y.z * x.y + m.z.z * x.z);
}

/// a simple 3x3 matrix 
typedef T3DMatrix<float> C3DFMatrix; 

NS_MIA_END

#endif
