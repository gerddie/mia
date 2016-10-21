/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef __nr2dmatrix_h
#define __nr2dmatrix_h


#include <mia/2d/datafield.hh>

NS_MIA_BEGIN

class TNR2DMatrix {
	double a_b;
	double a_;
	double b_4;
	double inv_nenner;
	C2DBounds Size;
	unsigned int internalDim;
	
public:	
	TNR2DMatrix(const C2DBounds& _ImgSize,double a,double b);
	
	double GetPraecondFact()const;
	template <class vector> vector operator *(const vector& v)const;
};


inline TNR2DMatrix::TNR2DMatrix(const C2DBounds& _ImgSize,double a,double b):
	Size(_ImgSize)
{
	double nenner = 4 * a + 2 * b;
	assert(nenner != 0.0);
	inv_nenner = 1/ nenner;
	a_b = (a+b)*inv_nenner;
	a_ = a * inv_nenner;
	b_4 = 0.25 * b * inv_nenner;
	internalDim = unsigned(Size.x) * unsigned(Size.y);
}

inline double  TNR2DMatrix::GetPraecondFact()const
{
	return inv_nenner;
}


template <class vector> vector TNR2DMatrix::operator *(const vector& v)const
{
	assert(v.size() == 2 * internalDim);
	
	vector Result(v); // Copy so we have already the mult with main diag  
	
	for (unsigned int i= 0; i< internalDim; i++) {
		float xz = 0;
		float yz = 0;
		
		if (i > 0) {
			xz -= v[i-1];
			yz -= v[i+internalDim-1];
		}
		if (i< internalDim-1){
			xz -= v[i+1];
			yz -= v[i+internalDim+1];
		}
		Result[i] += a_b * xz;
		Result[i+internalDim] += a_b * yz;
		
		xz = yz = 0;
		
		if (i > Size.x) {
			xz -= a_ * v[i-Size.x];
			yz -= a_ * v[i+internalDim-Size.x];
		}
		if (i< internalDim-Size.x){
			xz -= a_ * v[i+Size.x];
			yz -= a_ * v[i+internalDim+Size.x];
		}
		Result[i] += a_ * xz;
		Result[i+internalDim] += a_ * yz;
		xz = yz = 0;
		
		if (i > Size.x + 1){
			xz -= v[i + internalDim-Size.x - 1];
			yz -= v[i - Size.x - 1];
		}
		
		if (i > Size.x - 1){
			xz += v[i + internalDim-Size.x + 1];
			yz += v[i - Size.x + 1];
		}
		
		if (i < internalDim - Size.x - 1){
			xz -= v[i + internalDim + Size.x + 1];
			yz -= v[i + Size.x + 1];
		}
		if (i < internalDim - Size.x + 1){
			xz += v[i + internalDim + Size.x - 1];
			yz += v[i + Size.x - 1];
		}
		Result[i] += b_4 * xz;
		Result[i+internalDim] += b_4 * yz;
	}
	return Result;
}

#endif

/* CVS LOG

   $Log: NR2DMatrix.hh,v $
   Revision 1.2  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

NS_MIA_END
