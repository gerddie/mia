/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: 2DVector.hh 841 2006-02-15 16:58:25Z write1 $

/*! \brief A templated 2D vector class

  The tempate class T2DVector provides a a class for two-dimentional vectors
  The element-class T must provide the following functions/operators:
  Copy-contructor T(const T& org)
  Standard constructor T() should generate the element which is not
  allowed to be a divisor
  operators +, -, *, /, +=, -=, *=, /=, ==, != 

\file 2DVector.hh
\author Gert Wollny <wollny@cbs.mpg.de>, 2004

*/

#ifndef MIA_2D_VECTOR_HH
#define MIA_2D_VECTOR_HH
 
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <boost/lambda/lambda.hpp>

// MIA specific
#include <mia/core/defines.hh>

NS_MIA_BEGIN

template <class T > class T2DVector {
public:	

	typedef T value_type; 
	T x,y;
		
	// Contructors

	T2DVector():x(T()),y(T()){}

	T2DVector(int dim):x(T()),y(T()){
		assert(dim ==2);
	}

	T2DVector(T _x, T _y):x(_x),y(_y){};

	template <typename In>
	T2DVector(const T2DVector<In>& in):
		x(in.x), 
		y(in.y)
		{
		}
	// Functions

	T norm2() const {
		return T(x * x + y * y);
	}

	T norm() const {
		return T(sqrt(norm2()));
	}
	
	// Operators
	T2DVector& operator += (const T2DVector& a){
		x += a.x; y += a.y;
		return *this;
	}
	T2DVector& operator -= (const T2DVector& a){
		x -= a.x; y -= a.y;
		return *this;
	}
	T2DVector& operator *= (double a){
		x *= a; y *= a;
		return *this;
	}

	T2DVector& operator *= (const T2DVector& a){
		x *= a.x; y *= a.y;
		return *this;
	}

	T2DVector& operator /= (const T2DVector& a){
		if ( a.x == 0.0 || a.y == 0.0)
			throw std::invalid_argument("T2DVector<T>::operator /=: division by zero not allowed"); 
		x /= a.x; y /= a.y;
		return *this;
	}

	T2DVector& operator /= (double a){
		if ( a == 0.0 )
			throw std::invalid_argument("T2DVector<T>::operator /=: division by zero not allowed"); 
		x /= a; y /= a;
		return *this;
	}
	size_t size() const {
		return 2;
	}
	
	T& operator [](int i) {
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		default: {
			assert(!"access to value outside of range"); 
		}
		}
	}
	
	const T& operator [](int i) const {
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		default: {
			assert(!"access to value outside of range"); 
		}
		}
	}

	
	// Comparsion
	bool operator == (const T2DVector& a)const{
		return (x == a.x && y == a.y);
	}
	bool operator != (const T2DVector& a)const{
		return (! (*this == a));
	}

	void print(std::ostream& os) const {
		os << "<" << x << "," << y << ">"; 
	}
	
	void read(std::istream& is) {
		char c; 
		
		T r,s; 
		is >> c;
		if (c == '<') {
			is >> r;
			is >> c; 
			if (c != ',') {
				is.clear(std::ios::badbit);
				return; 
			}
			is >> s; 
			is >> c; 
			if (c != '>') {
				is.clear(std::ios::badbit);
				return; 
			}
			x = r; 
			y = s; 
		}else
			is.putback(c);
	}

};



template <typename T>
std::ostream& operator << (std::ostream& os, const T2DVector<T>& a)
{
	a.print(os); 
	return os; 
}

template <typename T>
std::istream& operator >> (std::istream& is, T2DVector<T>& a)
{
	a.read(is); 
	return is; 
}

template <typename  T>
T2DVector<T> operator +(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r += b; 
	return r; 
}

template <typename  T>
T2DVector<T> operator *(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r *= b; 
	return r; 
}

template <typename  T>
T2DVector<T> operator /(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r /= b; 
	return r; 
}


template <typename  T>
T2DVector<T> operator -(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r -= b; 
	return r; 
}

template <typename  T>
T dot(const T2DVector<T>& a, const T2DVector<T>& b)
{
	return b.x * a.x + b.y * a.y; 
}

template <typename  T>
T2DVector<T> operator / (const T2DVector<T>& a, T f)
{
	T2DVector<T> r(a); 
	r /= f; 
	return r; 
}

template <typename  T>
T2DVector<T> operator * (const T2DVector<T>& a, double f)
{
	T2DVector<T> r(a); 
	r *= f; 
	return r; 
}

template <typename  T>
T2DVector<T> operator * (double f, const T2DVector<T>& a)
{
	return a * f; 
}

template <typename T, typename S>
bool operator < (const T2DVector<T>& a, const T2DVector<S>& b) 
{
	return a.x < b.x && a.y < b.y; 
}

template <typename T, template <typename> class Vector> 
struct cross_product {
	typedef T return_type; 
	static return_type apply(const Vector<T>& a, const Vector<T>& b) {
		return a.x * b.y - a.y * b.x;
	}; 
}; 

template <typename  T>
T cross(const T2DVector<T>& a, const T2DVector<T>& b)
{
	return cross_product<T, T2DVector>::apply(a,b); 
}



/// float valued 2D vector
typedef T2DVector<float>    C2DFVector;

/// double valued 2D vector
typedef T2DVector<double>   C2DDVector;

/// unsigned int valued 2D vector - used as 2D size parameter
typedef T2DVector<unsigned int>   C2DBounds;


NS_MIA_END

namespace boost { 
	namespace lambda {
		
		template<class Act> 
		struct plain_return_type_2<arithmetic_action<Act>, mia::C2DFVector, mia::C2DFVector > {
			typedef mia::C2DFVector type;
		};
		template<> 
		struct plain_return_type_2<arithmetic_action<multiply_action>, mia::C2DFVector, float> {
			typedef mia::C2DFVector type;
		};
		
	}
}






#endif

