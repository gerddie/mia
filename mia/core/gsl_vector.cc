/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/gsl_vector.hh>

#include <cstring>

namespace gsl {

Vector::Vector():
	data(NULL), 
	cdata(NULL), 
	owner(false)
{
}

Vector::Vector(size_type n, bool clear):
	owner(true)
{
	cdata = data = (vector_type*) (clear ? gsl_vector_calloc (n) : gsl_vector_alloc (n));  
}

Vector::Vector(size_type size, const double *init):
	owner(true)
{
	cdata = data = (vector_type*) (gsl_vector_alloc (size));  
	memcpy(gsl_vector_ptr (data,0), init, size * sizeof(double)); 
}

Vector::Vector(const Vector& other):
	owner(true)
{
	assert(other.cdata); 
	
	cdata = data = gsl_vector_alloc (other.cdata->size);
	gsl_vector_memcpy(data, other.cdata); 
}

Vector& Vector::operator = (const Vector& other)
{
	if (&other != this) {
		vector_type *d =  gsl_vector_alloc(other.data->size); 
		gsl_vector_memcpy(d, other.cdata); 
		if (owner)  
			gsl_vector_free(data); 
		cdata = data = d; 
		owner = true; 
	}
	return *this; 
}

Vector::Vector(vector_type *holder):
	data(holder), 
	cdata(holder),
	owner(false)
{
}

Vector::Vector(const vector_type *holder):
	data(NULL), 
	cdata(holder),
	owner(false)
{
}

Vector::~Vector()
{
	if (owner) 
		gsl_vector_free(data);
}

const Vector::vector_type * Vector::operator  ->() const
{
	return cdata; 
}

Vector::vector_type * Vector::operator  ->()
{
	return data; 
}

void Vector::print(std::ostream& os) const
{
	os << "["; 
	std::copy(begin(), end(), std::ostream_iterator<double>(os, ", ")); 
	os << "]"; 
}

Vector::operator Vector::vector_pointer_type ()
{
	return data; 
}

Vector::operator Vector::vector_const_pointer_type () const
{
	return cdata; 
}

Vector::iterator Vector::begin()
{
	assert(data); 
	return vector_iterator(data->data, data->stride); 
}

Vector::iterator Vector::end()
{
	assert(data); 
	return vector_iterator(data->data + data->size * data->stride, data->stride);
}


Vector::const_iterator Vector::begin()const
{
	assert(cdata); 
	return const_vector_iterator(cdata->data, cdata->stride); 
}


Vector::const_iterator Vector::end()const
{
	assert(cdata); 
	return const_vector_iterator(cdata->data + cdata->size * cdata->stride, cdata->stride); 
}


Vector::size_type Vector::size() const
{
	return this->cdata->size; 
}


#ifdef NDEBUG 
	class CTurnOffErrorHandler {
	public: 
		CTurnOffErrorHandler(); 
	}; 


	CTurnOffErrorHandler::CTurnOffErrorHandler() 
	{
		gsl_set_error_handler_off (); 
	}
		
	const CTurnOffErrorHandler gsl_turn_off_error_handler; 		
#endif 

}
