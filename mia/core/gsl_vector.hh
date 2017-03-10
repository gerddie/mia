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

#ifndef GSLPP_VECTOR_HH
#define GSLPP_VECTOR_HH


#include <iostream>
#include <mia/core/gsl_defines.hh>
#include <mia/core/gsl_iterator.hh>
#include <gsl/gsl_vector.h>

namespace gsl {

/**
    This is a wrapper class around the GSL vector type. It provides 
    a compatibility layer to make it possible to use STL algorithms and constructs.
    
*/
class  EXPORT_GSL Vector {

public: 
	typedef vector_iterator iterator; 
	typedef const_vector_iterator const_iterator; 
	typedef size_t size_type; 
	typedef double value_type; 
	typedef double& reference;
	typedef const double& const_reference; 
	typedef gsl_vector vector_type; 
	typedef gsl_vector *vector_pointer_type; 
	typedef const gsl_vector *vector_const_pointer_type; 
	
	/**
	   Construct an empty vector without allocating the GSL data structures
	 */
	Vector();

	/**
	   Construct a vector of given size
	   \param size 
	   \param clear if set to \a true set all values to zero at allocation 
	 */
	Vector(size_type size, bool clear);


	/**
	   Construct a vector of given size and initialize it with the given data 
	   \param size 
	   \param init double array that must be at least of size \a size 
	 */
	Vector(size_type size, const double *init);

	/**
	   Wrap a pre-constructed GSL vector. The passed GSL-vector will not be destroyed 
	   when the destructor is called. 
	   The values of the GSL vector can be changed 
	   \param holder the already allocated GSL vector 
	   
	*/
	Vector(gsl_vector *holder); 
	/**
	   Wrap a pre-constructed GSL vector. The passed GSL-vector will not be destroyed 
	   when the destructor is called. 
	   The values of the GSL vector can \a not be changed 
	   \param holder the already allocated GSL vector 
	*/
	Vector(const gsl_vector *holder); 

	/**
	   Copy constructor, does a deep copy of the internal data structures. 
	 */
	Vector(const Vector& other); 

	/**
	   Move  constructor, does move the internal data structure to the new object. 
	 */
	Vector(Vector&& other); 
	
	
	/**
	   Copy operator, does a deep copy of the internal data structures. 
	 */
	Vector& operator = (const Vector& other); 

	/**
	   Move operator, does move the internal data structure to the new object. 
	 */
	Vector& operator = (Vector&& other); 

	
	/// Destructor 
	~Vector(); 
	
	/** \returns an STL compatible read-write iterator to the vector data 
	    pointing to the beginning 
	 */
	iterator begin(); 

	/** \returns an STL compatible read-write iterator to the vector data 
	    pointing to the past end 
	 */
	iterator end(); 

	/** \returns an STL compatible read-only iterator to the vector data 
	    pointing to the beginning 
	 */
	const_iterator begin()const; 

	/** \returns an STL compatible read-only iterator to the vector data 
	    pointing to the past end 
	 */
	const_iterator end()const; 

	/**
	   \returns number of elements in the vector 
	 */ 
	size_type size() const; 

	/**
	   Element read only access operator 
	   \param i 
	   \returns value 
	 */
	value_type operator[](size_t i) const {
	        assert(cdata); 
		return cdata->data[i * cdata->stride]; 
        }; 

	/**
	   Element read only access operator 
	   \param i 
	   \returns reference to value 
	 */
	reference operator[](size_t i) {
		assert(data); 
		return data->data[i * data->stride]; 
	}

	/// \returns transparent read-only access to underlying gsl_vector structure
	const gsl_vector * operator  ->() const; 
	
	/// \returns transparent read-write access to underlying gsl_vector structure
	gsl_vector * operator  ->(); 
	
        /// vector const pointer type operator  to enable transparent calls to the GSL APL
	operator Vector::vector_const_pointer_type () const; 

	/// vector pointer type operator  to enable transparent calls to the GSL APL
	operator Vector::vector_pointer_type (); 
	
	/**
	   Write the vector to a stream 
	   \param os the output stream 
	*/
	void print(std::ostream& os) const;

	bool is_writable() const;
	bool is_valid() const; 

protected:
	
	void reset_holder(gsl_vector *holder){
		cdata = data = holder; 
		owner = false; 
	}
	
	void reset_holder(const gsl_vector *holder){
		cdata = holder;
		owner = false;  
	}

private: 
	gsl_vector *data; 
	const gsl_vector *cdata; 
	bool owner; 
}; 


inline Vector operator + (const Vector& lhs, const Vector& rhs) 
{
	Vector result(lhs); 
	gsl_vector_add(result, rhs); 
	return result; 
}

inline Vector operator - (const Vector& lhs, const Vector& rhs) 
{
	Vector result(lhs); 
	gsl_vector_sub(result, rhs); 
	return result; 
}

inline Vector operator * (const Vector& lhs, double f) 
{
	Vector result(lhs); 
	gsl_vector_scale(result, f); 
	return result; 
}

/**
   Wrapper for the gsl_vector_view providing transparent access 
   to the underlying vector. 
*/
class EXPORT_GSL VectorView :public Vector {
public: 
	VectorView(gsl_vector_view vv): m_view(vv) 
	{
		reset_holder(&m_view.vector); 
	}; 
private: 
	gsl_vector_view m_view; 
}; 


/**
   Wrapper for the gsl_vector_const_view providing transparent access 
   to the underlying vector. 
*/
class EXPORT_GSL ConstVectorView {
public: 
	ConstVectorView(gsl_vector_const_view vv):m_view(vv), 
		m_holder(&m_view.vector)
	{
		
	}; 
	
	Vector::const_iterator begin()const{
		return m_holder.begin(); 
	}
		
	Vector::const_iterator end()const{
		return m_holder.end(); 
	}
	
	Vector::size_type size() const {
		return m_holder.size(); 
	}
	
	double operator[](size_t i)const {
		
		return m_holder[i]; 
	}; 

	operator const Vector&(){
		return m_holder; 
	}
		
	const gsl_vector * operator  ->() const{
		return m_holder.operator ->(); 
	}
	
	operator Vector::vector_const_pointer_type () const {
		return m_holder.operator Vector::vector_const_pointer_type(); 
	}

private: 
	gsl_vector_const_view m_view; 
	const Vector m_holder; 
}; 

inline std::ostream& operator << (std::ostream& os, const Vector& v) {
	v.print(os); 
	return os; 
} 

}

#endif


