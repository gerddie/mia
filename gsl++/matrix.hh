/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010 Gert Wollny 
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
namespace gsl {

class Matrix {
public: 
	Matrix(); 
	Matrix(size_t rows, size_t columns, bool clean); 
	Matrix(const Matrix& other); 
	Matrix& operator =(const Matrix& other); 

	~Matrix(); 

	size_t rows()const; 
	size_t cols()const; 
	
	
	void set(size_t i, size_t j, double x); 
	double operator ()(size_t i, size_t j) const; 

	operator gsl_matrix *(); 
	operator const gsl_matrix *() const; 

private: 
	gsl_matrix *_M_matrix; 
}; 

} // end namespace 

#endif
