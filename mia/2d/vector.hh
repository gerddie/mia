/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef MIA_2D_VECTOR_HH
#define MIA_2D_VECTOR_HH
 
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <istream>
#include <iomanip>
#include <type_traits>

// MIA specific
#include <mia/core/type_traits.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/attributetype.hh>

NS_MIA_BEGIN

/**
   \ingroup basic
   \brief a 2D vector 
   
   A 2D vector that provides the usual set of operations required to handle such objects. 
   \tparam type of the elements 
 */
template <class T > class T2DVector {
public:	

	/// typedef for generic access to the element type 
	typedef T value_type; 
	
	/// first element 
	T x;
	
	/// second element 
	T y;
	
	/// a static for the value <1,1>. 
	static const T2DVector<T> _1; 

	/// a static for the value <0,0>. 
	static const T2DVector<T> _0; 
		
	T2DVector():x(T()),y(T()){}

	/** this constructor is required for some generic code 
	    It is explicit to avoid some problems with automatic assigment 
	 */ 
	explicit T2DVector(int dim):x(T()),y(T()){
		assert(dim ==2);
	}

	/**
	   Contruct the vector with the according elements 
	   \param _x 
	   \param _y 
	 */
	T2DVector(T _x, T _y):x(_x),y(_y){};

	
	/**
	   Automatically convert a 2D vector from another element type 
	 */
	template <typename In>
	T2DVector(const T2DVector<In>& in):
		x(in.x), 
		y(in.y)
		{
		}
	
	// Functions


	///@returns square norm of the vector 
	T norm2() const {
		return T(x * x + y * y);
	}

	///@returns norm of the vector 
	T norm() const {
		return T(sqrt(norm2()));
	}
	
	///@returns product of the elements of the vector  
	double product() const {
		return x * y; 
	}

	// Operators 

	/// in place addition 
	T2DVector& operator += (const T2DVector& a){
		x += a.x; y += a.y;
		return *this;
	}

	/// in place subtraction 
	T2DVector& operator -= (const T2DVector& a){
		x -= a.x; y -= a.y;
		return *this;
	}

	/// in place multiplication with a scalar 
	T2DVector& operator *= (double a){
		x *= a; y *= a;
		return *this;
	}

	/// in place element wise multiplication of two 2D vectors 
	T2DVector& operator *= (const T2DVector& a){
		x *= a.x; y *= a.y;
		return *this;
	}

	/// in place element wise division of two 2D vectors 
	T2DVector& operator /= (const T2DVector& a){
		if ( a.x == 0.0 || a.y == 0.0)
			throw std::invalid_argument("T2DVector<T>::operator /=: division by zero not allowed"); 
		x /= a.x; y /= a.y;
		return *this;
	}

	/// in place division by a scalar  
	T2DVector& operator /= (double a){
		if ( a == 0.0 )
			throw std::invalid_argument("T2DVector<T>::operator /=: division by zero not allowed"); 
		x /= a; y /= a;
		return *this;
	}

	/// returns the size of this vector, always 2
	size_t size() const {
		return 2;
	}
	
	/** Array access operator for some generic code 
	    One should have a look how expensive this function is 
	 */ 
	T& operator [](int i) {
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		default: {
			DEBUG_ASSERT_RELEASE_THROW(false, "access to value at (", i, ") outside of range (0-1)"); 
		}
		}
	}
	
	/** Array access operator for some generic code 
	    One should have a look how expensive this function is 
	 */ 
	const T& operator [](int i) const {
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		default: {
			DEBUG_ASSERT_RELEASE_THROW(false, "access to value at (", i, ") outside of range (0-1)");
		}
		}
	}
	
	/// fill all the elements with the given value
	void fill(T v) {
		x = y = v; 
	}
	
	/// Equal operator 
	bool operator == (const T2DVector& a)const{
		return (x == a.x && y == a.y);
	}
	
	/// not equal operator 
	bool operator != (const T2DVector& a)const{
		return (! (*this == a));
	}

	/// print the vector to a stream with special formatting 
	void print(std::ostream& os) const {
		os  << x << "," << y; 
	}
	
	/// read the properly formatted 2D vector from a stream 
	void read(std::istream& is) {
		char c; 
		
		T r,s; 
		is >> c;
		// if we get the opening delimiter '<' then we also expect the closing '>'
		// otherwise just read two coma separated values. 
		// could use the BOOST lexicel cast for better error handling
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
		}else {
			is.putback(c); 
			is >> r;
			is >> c; 
			if (c != ',') {
				is.clear(std::ios::badbit);
				return; 
			}
			is >> s; 
			x = r; 
			y = s; 
		}
		
	}

};


struct EAttributeType_2d : public EAttributeType {
	
	static const int vector_2d_bit = 0x20000; 
	
	static bool is_vector(int type) {
		return type & vector_2d_bit; 
        }
}; 

template <typename T> 
struct attribute_type<T2DVector<T>> : public EAttributeType_2d {
        static const int value = attribute_type<T>::value | vector_2d_bit;
}; 


/// @cond NEVER  

template <typename T> 
struct atomic_data<T2DVector<T> > {
	typedef T type;
	static const int size; 
}; 

template <typename T> 
const int atomic_data<T2DVector<T> >::size = 2; 

/// @endcond 

template <typename T> 
const T2DVector<T> T2DVector<T>::_1 = T2DVector<T>(1,1); 

template <typename T> 
const T2DVector<T> T2DVector<T>::_0 = T2DVector<T>(0,0); 

/**
   operator to write a 2D vector to a stream 
   \tparam type of the vector values 
   \param os output stream 
   \param a vector 
   \returns reference to stream to allow chaining of the operator 
 */
template <typename T>
std::ostream& operator << (std::ostream& os, const T2DVector<T>& a)
{
	a.print(os); 
	return os; 
}

/**
   operator to read a 2D vector from a stream 
   \tparam type of the vector values 
   \param is input stream 
   \param a vector 
   \returns reference to stream to allow chaining of the operator 
 */
template <typename T>
std::istream& operator >> (std::istream& is, T2DVector<T>& a)
{
	a.read(is); 
	return is; 
}

/**
   Add operator for two 2D vectors that hold the same data type 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns a+b
 */
template <typename  T>
T2DVector<T> operator +(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r += b; 
	return r; 
}

/**
   Add operator for two 2D vectors that hold different data types 
   Target type is taken from the lhs operator 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns a+b
 */
template <typename  T, typename  S>
T2DVector<T> operator +(const T2DVector<T>& a, const T2DVector<S>& b)
{
	return T2DVector<T>(a.x + b.x, a.y + b.y); 
}

/**
   Element wise multiplication  operator for two 2D vectors that hold the same data type 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns <a.x*b.x, a.y*b.y>
 */
template <typename  T>
T2DVector<T> operator *(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r *= b; 
	return r; 
}

/**
   Element wise division  operator for two 2D vectors that hold the same data type. 
   Throws a std::invalid_argument exception if b.x=== or b.y==0. 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns <a.x/b.x, a.y/b.y>
 */
template <typename  T>
T2DVector<T> operator /(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r /= b; 
	return r; 
}

/**
   Element wise subtraction operator for two 2D vectors that hold the same data type. 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns a-b
 */

template <typename  T>
T2DVector<T> operator -(const T2DVector<T>& a, const T2DVector<T>& b)
{
	T2DVector<T> r(a); 
	r -= b; 
	return r; 
}

/**
   dot product for two 2D vectors that hold the same data type. 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns a.x*b.x + a.y*b.y 
*/

template <typename  T>
T dot(const T2DVector<T>& a, const T2DVector<T>& b)
{
	return b.x * a.x + b.y * a.y; 
}

/**
   Division of a vector by a scalar. 
   Throws a std::invalid_argument exception if f==0. 
   \tparam type of the vector values 
   \param a 
   \param f
   \returns <a.x/f, b.y/f>
*/

template <typename  T>
T2DVector<T> operator / (const T2DVector<T>& a, double f)
{
	T2DVector<T> r(a); 
	r /= f; 
	return r; 
}

/**
   Multiplication of a vector with a scalar. 
   \tparam type of the vector values 
   \param a  vector 
   \param f scalar 
   \returns <a.x*f, b.y*f>
*/
template <typename  T>
T2DVector<T> operator * (const T2DVector<T>& a, double f)
{
	T2DVector<T> r(a); 
	r *= f; 
	return r; 
}

/**
   Multiplication of a scalar with a vector. 
   \tparam type of the vector values 
   \param f scalar 
   \param a vector 
   \returns <a.x*f, b.y*f>
*/
template <typename  T>
T2DVector<T> operator * (double f, const T2DVector<T>& a)
{
	return a * f; 
}

/**
   Comparison of a vector. Not the this less operator does not define 
   an order, since it is possible that a =/= b and !a<b && !b<a. 
   \param a 
   \param b 
   \returns true if both elements in a are less the the corresponding elements in b. 
 */
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

/**
   Cross product of two 2D vectors. Technically it's the 3D cross product with 
   the z-elements set to zero and ignoring all zero elements of the output vector 
   \param a
   \param b 
   \returns 2D cross product 
   \todo Check why does it use indirection to the apply-function? 
 */
template <typename  T>
T cross(const T2DVector<T>& a, const T2DVector<T>& b)
{
	return cross_product<T, T2DVector>::apply(a,b); 
}


template <typename T>
struct less_then<T2DVector<T> > {
	bool operator() (const T2DVector<T>& a, const T2DVector<T>& b) const {
		return a.y < b.y || (a.y == b.y && a.x < b.x);
	}
}; 


/// float valued 2D vector
typedef T2DVector<float>    C2DFVector;

/// double valued 2D vector
typedef T2DVector<double>   C2DDVector;

/// unsigned int valued 2D vector - used as 2D size parameter
typedef T2DVector<unsigned int>   C2DBounds;


NS_MIA_END

#endif

