/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
 */



#ifndef mia_2d_divcurlmatrix_hh
#define mia_2d_divcurlmatrix_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/2DDatafield.hh>

NS_MIA_BEGIN

class  EXPORT_2D C2DDivCurlMatrix {
public:
	C2DDivCurlMatrix(const C2DBounds& size, const CBSplineKernel* kernel);
	~C2DDivCurlMatrix();
	double multiply(const C2DFVectorfield& coefficients) const;
	C2DFVectorfield multiply_for_gradient(const C2DFVectorfield& coefficients) const;
	double value_at(const C2DFVectorfield& coefficients, size_t m, size_t n) const;
	C2DFVector derivative_at(const C2DFVectorfield& coefficients, size_t m, size_t n) const;
private:
	struct C2DDivCurlMatrixImpl* impl;
};

NS_MIA_END

#endif
