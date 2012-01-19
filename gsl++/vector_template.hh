/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef GSL_VECTOR_TEMPLATE
#define GSL_VECTOR_TEMPLATE

#include <gsl/gsl_vector.h>
#include <cassert>

namespace gsl {


typedef unsigned long  ulong; 
typedef unsigned short ushort; 
typedef unsigned int   uint; 
typedef unsigned char  uchar; 


template <typename T> 
struct gsl_vector_dispatch {
	typedef void vector_type; 
	typedef T value_type;
	typedef T *iterator;	
	typedef const T *const_iterator;
	typedef size_t size_type;
	typedef T& reference;
	typedef const T& const_reference;

protected: 
	static vector_type *alloc(size_t n);
	static vector_type *alloc_from(vector_type *other);
	static vector_type *calloc(size_t n);
	static void gsl_free(vector_type *v);
	static value_type get(const vector_type *v, size_t i);
}; 


/* because of the C-nameing scheme we need specialications for ceach type */
#define GSL_VECTOR_DISPATCH(TYPE)		 \
	template <>						\
	struct gsl_vector_dispatch<TYPE> {			\
		typedef TYPE value_type;			\
		typedef TYPE *iterator;				\
		typedef const TYPE *const_iterator;		\
		typedef size_t size_type;			\
		typedef TYPE& reference;			\
		typedef const TYPE& const_reference;		\
								\
		typedef gsl_vector_##TYPE vector_type;		\
		typedef gsl_vector_##TYPE *vector_pointer_type; \
	protected:							\
	static vector_type *alloc(size_t n) {			\
		return gsl_vector_##TYPE##_alloc(n);			\
	}								\
	static vector_type *calloc(size_t n) {			\
		return gsl_vector_##TYPE##_calloc(n);			\
	}								\
	static vector_type *alloc_from(vector_type *other) {	\
		return gsl_vector_##TYPE##_alloc_from_vector(other, 0,	\
							     other->size, 1); \
	}								\
	static void free(vector_type *v) {				\
		gsl_vector_##TYPE##_free(v);				\
	}								\
	static value_type get(const vector_type *v, size_t i) {		\
		return gsl_vector_##TYPE##_get(v,i);			\
	}								\
	}; 
	
	GSL_VECTOR_DISPATCH(float); 
	GSL_VECTOR_DISPATCH(long); 
	GSL_VECTOR_DISPATCH(int); 
	GSL_VECTOR_DISPATCH(short); 
	GSL_VECTOR_DISPATCH(char); 
	GSL_VECTOR_DISPATCH(ulong); 
	GSL_VECTOR_DISPATCH(uint); 
	GSL_VECTOR_DISPATCH(ushort); 
	GSL_VECTOR_DISPATCH(uchar); 
	
	
/* double is a special case named differently */	
	template <>						
	struct gsl_vector_dispatch<double> {	
		
		typedef double value_type;		
		typedef double *iterator;			
		typedef const double *const_iterator;	
		typedef size_t size_type;		
		typedef double& reference;		
		typedef const double& const_reference;	
		
		typedef gsl_vector vector_type; 
		typedef gsl_vector *vector_pointer_type; 
		
		protected:						
		static vector_type *alloc(size_t n) {		
			return gsl_vector_alloc(n);		
		}							
		static vector_type *calloc(size_t n) {		
			return gsl_vector_calloc(n);		
		}							
		static vector_type *alloc_from(vector_type *other) { 
			return gsl_vector_alloc_from_vector(other, 0, 
							    other->size, 1); 
		}							
		static void free(vector_type *v) {			
			gsl_vector_free(v);			
		}							
		static value_type get(const vector_type *v, size_t i)  { 
			return gsl_vector_get(v,i);		
		}							
	}; 

	/**
	   This is a wrapper class around the GSL vector type. It provides 
	   a compatibility layer to make it possible to use STL algorithms and constructs.
	*/
template <typename T> 
class TVector : public gsl_vector_dispatch<T> {
public: 
	typedef typename gsl_vector_dispatch<T>::iterator iterator; 
	typedef typename gsl_vector_dispatch<T>::const_iterator const_iterator; 
	typedef typename gsl_vector_dispatch<T>::size_type size_type; 
	typedef typename gsl_vector_dispatch<T>::value_type value_type; 
	typedef typename gsl_vector_dispatch<T>::vector_type vector_type; 
	typedef typename gsl_vector_dispatch<T>::reference reference;
	typedef typename gsl_vector_dispatch<T>::const_reference const_reference; 
	typedef typename gsl_vector_dispatch<T>::vector_pointer_type vector_pointer_type; 
	
	/**
	   Construct an empty vector without allocating the GSL data structures
	 */
	TVector();

	/**
	   Construct a vector of given size
	   \param size 
	   \param clear if set to \a true set all values to zero at allocation 
	 */
	TVector(size_type size, bool clear=true);

	/**
	   Wrap a pre-constructed GSL vector. The passed GSL-vector will not be destroyed 
	   when the destructor is called. 
	   The values of the GSL vector can be changed 
	   \param holder the already allocated GSL vector 
	   
	*/
	TVector(vector_type *holder); 
	/**
	   Wrap a pre-constructed GSL vector. The passed GSL-vector will not be destroyed 
	   when the destructor is called. 
	   The values of the GSL vector can \a not be changed 
	   \param holder the already allocated GSL vector 
	*/
	TVector(const vector_type *holder); 

	/**
	   Copy constructor, does a deep copy of the internal data structures. 
	 */
	TVector(const TVector<T>& other); 

	/**
	   Copy operator, does a deep copy of the internal data structures. 
	 */
	TVector<T>& operator = (const TVector<T>& other); 

	~TVector(); 
	
	iterator begin(); 
	iterator end(); 

	const_iterator begin()const; 
	const_iterator end()const; 

	size_type size() const; 

	const value_type operator[](size_t i)const; 

	reference operator[](size_t i){
		assert(data); 
		return data->data[i]; 
	}

	/// read only vector pointer type operator to enable transparent calls to the GSL APL
	operator const vector_type *() const; 
	
	/// vector pointer type operator  to enable transparent calls to the GSL APL
	operator vector_pointer_type (); 
private: 
	vector_type *data; 
	const vector_type *cdata; 
	bool owner; 
}; 

}
#endif
