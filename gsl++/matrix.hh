/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011 Gert Wollny 
 *
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

#ifndef GSLPP_MATRIX_HH
#define GSLPP_MATRIX_HH


#include <gsl/gsl_matrix.h>
#include <gsl++/gsldefines.hh>

namespace gsl {



/**
   This is a wrapper class around the GSL matrix type. It provides 
   a compatibility to avoid handling de-alloction manually.
*/
class EXPORT_GSL Matrix {
public: 
	Matrix(); 

	/**
	   Create a matrix of size rows \f$\times\f$ columns, 
	   \param rows
	   \param columns 
	   \param clean allocate zeroing out all elements 
	 */
	Matrix(size_t rows, size_t columns, bool clean); 

	/**
	   Copy constructor that executes a deep copy 
	 */
	Matrix(const Matrix& other); 


	/**
	   Copy operator that executes a deep copy 
	 */
	Matrix& operator =(const Matrix& other); 

	~Matrix(); 

	size_t rows()const; 
	size_t cols()const; 
	
	void set(size_t i, size_t j, double x); 
	double operator ()(size_t i, size_t j) const; 
	
	operator gsl_matrix *(); 
	operator const gsl_matrix *() const; 

private: 
	gsl_matrix *m_matrix; 
}; 

} // end namespace 

#endif
