/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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
 */


#ifndef mia_2d_ppmatrix_hh
#define mia_2d_ppmatrix_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN

/**
   Precomputed Matrix for the DivCurl regularization. 
   This class stores the precomputed matrix for the divcurl regislarization 
   of a spline based transformation. 
*/



class EXPORT_2D C2DPPDivcurlMatrix {
public: 
	/**
	   Construct the matrix according to the given coefficient field size, the used B-Spline kernel
	   and the divergence and rotational weights
	   \param size coefficient field size
	   \param range full data range 
	   \param kernel B-Spline kernel used for the transformation 
	   \param wd weight for divergence 
	   \param wr weight for rotation (curl)
	 */

	C2DPPDivcurlMatrix(const C2DBounds& size, const C2DFVector& range, const CBSplineKernel& kernel, 
			   double wd, double wr); 

	~C2DPPDivcurlMatrix(); 

	/**
	   Given this matrix P and the coefficient field c evaluate the value for c^T P c 
	   \param coefficients B-Spline coefficient field c 
	   \returns <c^T, P, c>
	 */
	double operator * (const C2DFVectorfield& coefficients) const; 

	/**
	   Given this matrix P and the coefficient field c evaluate the value for c^T P c 
	   \param coefficients B-Spline coefficient field c 
	   \returns <c^T, P, c>
	 */
	double operator * (const T2DDatafield<C2DDVector>& coefficients) const; 


	/**
	   Given this matrix P and the coefficient field c evaluate the value for c^T P c 
	   \param coefficients B-Spline coefficient field c 
	   \retval gradinet gradient of the divcurl cost 
	   \returns <c^T, P, c>
	 */
	
	double evaluate(const C2DFVectorfield& coefficients, CDoubleVector& gradient) const; 
	double evaluate(const T2DDatafield<C2DDVector>& coefficients, CDoubleVector& gradient) const; 

	/**
	   \returns fild size this matrix was created for 
	 */
	const C2DBounds& get_size() const; 

	/**
	   re-initialize the matrix (if the values are changed)
	 */
	void reset(const C2DBounds& size, const C2DFVector& range, const CBSplineKernel& kernel, 
		   double wd, double wr); 
private: 
	struct C2DPPDivcurlMatrixImpl *impl; 

}; 
NS_MIA_END

#endif
