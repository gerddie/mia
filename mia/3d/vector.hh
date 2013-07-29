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

#ifndef __MIA_3DVECTOR_HH
#define __MIA_3DVECTOR_HH 1

#include <typeinfo>
#include <assert.h>
#include <stdexcept>
#include <math.h>
#include <complex>
#include <iostream>
#include <type_traits>

#include <mia/core/defines.hh>
#include <mia/core/type_traits.hh>
#include <mia/core/attributetype.hh>

NS_MIA_BEGIN

/**
   @ingroup basic 
   \brief A simple 3D vector type. 

   This class is the template for a 3D vector that provides support for some common operators. 
   \tparam T element type 
*/

template < class T > 
class T3DVector {
public:
	/// vector element
	T x;
	/// vector element
	T y;
	/// vector element
	T z;
	
	/// typedef for generic programming 
	typedef T value_type; 

	/// standart constructor 
	T3DVector():x(T()),y(T()),z(T()){};
	
	/// create a zero-vector, \a dim must be 3
	explicit T3DVector(int dim):x(T()),y(T()),z(T()){
		assert(dim == 3);
	}
	
        /// we provide the default copy mechanisms 
	T3DVector(const T3DVector<T>& other) = default; 
	
	/// we provide the default copy mechanisms 
	T3DVector<T>& operator = (const T3DVector<T>& other) = default; 

	/// constructor to construct vector from values
	T3DVector(const T& x_,const T& y_,const T& z_):
		x(x_),y(y_),z(z_){
	}
	
	/// type casting copy constructor
	template <class in> explicit T3DVector(const T3DVector<in>& org):
		x(T(org.x)),y(T(org.y)),z(T(org.z)){
	}
	
	/// assignment from different vector type
	template <class in> 
	T3DVector<T>& operator =(const T3DVector<in>& org){
		x=org.x; y=org.y; z=org.z;
		return *this;
	}
	
	//! square of Euclidian norm of the vector
	double norm2()const{
		return x * x + y * y + z * z;
	}
	
	/// \returns the product of the vector components
	double product() const {
		return x * y * z; 
	}
	/// \returns the Euclidian norm of the vector
	double norm()const{
		return sqrt(norm2());
	}
	
	/// \returns the dimension of vector (always 3)
	int size() const {
		return 3;
	}

	/// Fill the vector elements with value v 
	void fill(T v) {
		x = y = z = v;
	}

	/**
	   Implement the const operator [] for this type of vector 
	   \param i index 
	   \returns value at index 
	   \remark the performance of this needs to be analyzed. The compiler should be able 
	   to translate this to a simple memory access.  
	 */

	const T operator [](size_t i) const {
		assert(i < 3); 
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		case 2:return z; 
		default: 
			throw std::logic_error("Access to vectorelement out of range"); 
		}
	}

	/**
	   Implement the writable operator [] for this type of vector 
	   \param i index 
	   \returns reference value at index 
	   \remark the performance of this needs to be analyzed. The compiler should be able 
	   to translate this to a simple memory access.  
	 */

	T& operator [](size_t i) {
		assert(i < 3); 
		switch (i) {
		case 0:return x; 
		case 1:return y; 
		case 2:return z; 
		default: 
			throw std::logic_error("Access to vectorelement out of range"); 
		}
	}
	
	/// inplace addition 
	T3DVector<T>& operator +=(const T3DVector<T>& a){
		x+=a.x; y+=a.y; z+=a.z;
		return *this;
	}
	
	/// inplace subtraction
	T3DVector<T>& operator -=(const T3DVector<T>& a){
		x-=a.x; y-=a.y; z-=a.z;
		return *this;
	}
	
	/// inplace multiplication 
	T3DVector<T>& operator *=(const double a){
		x = T(x * a); y = T(y * a); z = T(z * a);
		return *this;
	}

	/// inplace component wise multiplication 
	T3DVector<T>& operator *=(const T3DVector<T>& a){
		x = T(x * a.x); y = T(y * a.y); z = T(z * a.z);
		return *this;
	}

	
	/// inplace divisison by a scalar 
	T3DVector<T>& operator /=(const double a){
		assert(a != 0.0);
		x = T(x/ a); y =T (y / a); z = T(z / a);
		return *this;
	}

	/// print out the formatted vector to the stream
	void write(std::ostream& os)const {
		os  << x << "," << y << "," << z; 
	}

	/// read the vector from a formatted string 
	void read(std::istream& is) {
		char c; 
		
		T r,s,t; 
		is >> c;
		// if we get the opening delimiter '<' then we also expect the closing '>'
		// otherwise just read three coma separated values. 
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
			if (c != ',') {
				is.clear(std::ios::badbit);
				return; 
			}
			is >> t; 
			is >> c; 
			if (c != '>') {
				is.clear(std::ios::badbit);
				return; 
			}
			x = r; 
			y = s; 
			z = t;
		}else{
			is.putback(c); 
			is >> r;
			is >> c; 
			if (c != ',') {
				is.clear(std::ios::badbit);
				return; 
			}
			is >> s; 
			is >> c; 
			if (c != ',') {
				is.clear(std::ios::badbit);
				return; 
			}
			is >> t; 
			x = r; 
			y = s; 
			z = t;
		}
	}

	/// swizzle operator 
	const T3DVector<T>&  xyz()const {
		return *this; 
	}
	
	/// swizzle operator 
	const T3DVector<T> xzy()const {
		return T3DVector<T>(x,z,y); 
	}

	/// swizzle operator 
	const T3DVector<T> yxz()const {
		return T3DVector<T>(y,x,z); 
	}

	/// swizzle operator 
	const T3DVector<T> yzx()const {
		return T3DVector<T>(y,z,x); 
	}

	/// swizzle operator 
	const T3DVector<T> zyx()const {
		return T3DVector<T>(z,y,x); 
	}
	
	/// swizzle operator 
	const T3DVector<T> zxy()const {
		return T3DVector<T>(z,x,y); 
	}
	
	/// declare the vector (1,1,1)
	static T3DVector<T> _1; 

	/// declare the vector (0,0,0)
	static T3DVector<T> _0; 
	
	/// the number of elements this vector holds (=3)
	static const unsigned int  elements; 
};


struct EAttributeType_3d : public EAttributeType {
	
	static const int vector_3d_bit = 0x40000; 
	
	static bool is_vector3d(int type) {
		return type & vector_3d_bit; 
        }
}; 

template <typename T> 
struct attribute_type<T3DVector<T>> : public EAttributeType_3d {
        static const int value = attribute_type<T>::value | vector_3d_bit;
}; 


/// @cond NEVER  
template <typename T> 
struct atomic_data<T3DVector<T> > {
	typedef T type; 
	static const int size; 
}; 

template <typename T> 
const int atomic_data<T3DVector<T> >::size = 3; 
/// @endcond 


/**
   Cross product of two 3D vectors 
   \param a
   \param b
   \returns cross product a x b 
 */

template <typename T>
T3DVector<T> cross(const T3DVector<T>& a, const T3DVector<T>& b) 
{
	return T3DVector<T>( 
		a.y * b.z - b.y * a.z, 
		a.z * b.x - b.z * a.x, 
		a.x * b.y - b.x * a.y
		); 
}


/// A way to get the norm of a T3DVector using \a faba syntax
template <class T> double fabs(const T3DVector<T>& t)
{
	return t.norm();
}

/// \returns the dot product of input vectors a and b
template <class T> double dot(const T3DVector<T>& a, const T3DVector<T>& b)
{
	return a.x * b.x + a. y * b.y + a.z * b.z;
}


/// A float 3D Vector
typedef T3DVector<float>    C3DFVector;

/// A double 3D Vector
typedef T3DVector<double>   C3DDVector;

/// A unsinged int 3D Vector (used for 3D field sizes)
typedef T3DVector<unsigned int>   C3DBounds;



/// stream output operator for 3DVector
 template <class T> 
std::ostream& operator << (std::ostream& os, const T3DVector<T>& v)
{
	v.write(os);
	return os; 
}

/// stream input operator for 3DVector
template <class T> 
std::istream& operator >> (std::istream& is, T3DVector<T>& v)
{
	v.read(is);
	return is; 
}


/// vector addition
template <class T> 
inline const T3DVector<T> operator +(const T3DVector<T>& a,const T3DVector<T>& b){
	T3DVector<T> tmp(a);
	tmp += b; 
	return tmp;
}

/**
   Add operator for two 3D vectors that hold different data types 
   Target type is taken from the lhs operator 
   \tparam type of the vector values 
   \param a 
   \param b
   \returns a+b
 */
template <typename  T, typename  S>
T3DVector<T> operator +(const T3DVector<T>& a, const T3DVector<S>& b)
{
	return T3DVector<T>(a.x + b.x, a.y + b.y, a.z + b.z); 
}

		
/// vector subtraction
template <class T> 
inline const T3DVector<T> operator -(const T3DVector<T>& a,const T3DVector<T>& b){
	T3DVector<T> tmp(a);
	tmp -= b; 
	return tmp;
}

/// vector scalar product
template <class T> 
inline const T3DVector<T> operator *(const T3DVector<T>& a,const T3DVector<T>& b)
{
	return T3DVector<T>(b.x * a.x, b.y * a.y, b.z * a.z);
}

/// vector division by scalar
template <class T>
inline const T3DVector<T> operator /(const T3DVector<T>& a,double f)
{
	assert(f != T());
	T3DVector<T> tmp (a);
	tmp /= f; 
	return tmp;
}

/**
   component wise division of two vectors 
 */

template <class T>
inline const T3DVector<T> operator / (const T3DVector<T>& a, const T3DVector<T>& b)
{
	assert(b.x != 0.0 && b.y != 0.0 && b.z != 0.0);
	return T3DVector<T>(a.x/b.x, a.y/b.y, a.z/b.z); 
}


/// vector multiplication with scalar
template <class T> 
inline const T3DVector<T> operator *(const T3DVector<T>& a, double f)
{
	T3DVector<T> tmp (a);
	tmp *= f;
	return tmp;
}


/// vector multiplication with scalar reverse order
template <class T> 
inline const T3DVector<T> operator *(double f, const T3DVector<T>& a)
{
	return a * f; 
}


/// 3D vector cross product
template <class T> 
inline const T3DVector<T> operator ^(const T3DVector<T>& a,const T3DVector<T>& b)
{
	return T3DVector<T>( a.y * b.z - b.y * a.z,
			     a.z * b.x - b.z * a.x,
			     a.x * b.y - b.x * a.y);
}

/// comparison operator equal
template <class T> 
inline bool operator == (const T3DVector<T>& a,const T3DVector<T>& b)
{
	return (b.x == a.x && b.y == a.y && b.z == a.z);
}

/// comparison operator not equal
template <class T>
inline bool operator != (const T3DVector<T>& a,const T3DVector<T>& b)
{
	return ! (a == b);
}

/// comparison less, returns true if all components of \a a are less then those of  \a b
template <class T>
bool operator <  (const T3DVector<T>& a,const T3DVector<T>& b){
	return (a.x < b.x && a.y < b.y && a.z < b.z);
}

/// comparison less or equal, returns true if all components of \a a are less or equal then those of  \a b
template <class T>
bool operator <= (const T3DVector<T>& b, const T3DVector<T>& a){
	return (b.x <= a.x && b.y <= a.y && b.z <= a.z);
}

/// comparison greater, returns true if all components of \a a are greater then those of  \a b
template <class T>
bool operator >  (const T3DVector<T>& b, const T3DVector<T>& a){
	return (b.x > a.x && b.y > a.y && b.z > a.z);
}

/// comparison greater or equal, returns true if all components of \a a are greater or equal then those of  \a b
template <class T>
bool operator >= (const T3DVector<T>& b, const T3DVector<T>& a){
	return (b.x >= a.x && b.y >= a.y && b.z >= a.z);
}
template <typename T >
T3DVector<T> T3DVector<T>::_1 = T3DVector<T>(1,1,1);

template <typename T >
T3DVector<T> T3DVector<T>::_0 = T3DVector<T>(0,0,0);

template <typename T>
struct less_then<T3DVector<T> > {
	bool operator() (const T3DVector<T>& a, const T3DVector<T>& b) const{
		return a.z < b.z || 
			(a.z ==  b.z && 
			 (a.y < b.y || (a.y == b.y && a.x < b.x))); 
	}
}; 



NS_MIA_END


#endif
