/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_2d_ppmatrix_hh
#define mia_2d_ppmatrix_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/datafield.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 
   @brief Precomputed matrix for the DivCurl regularization. 

   Precomputed Matrix for the DivCurl regularization. 
   This class stores the precomputed matrix for the divcurl regislarization 
   of a spline based transformation. 
   @todo Optimize like the 3D version that only stores the patch of required values instead 
   of building the whole diagonals 
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

	C2DPPDivcurlMatrix(const C2DBounds& size, const C2DFVector& range, const CSplineKernel& kernel, 
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
	   \param[out] gradient gradient of the divcurl cost 
	   \returns <c^T, P, c>
	 */
	
	double evaluate(const C2DFVectorfield& coefficients, CDoubleVector& gradient) const; 

	/**
	   Given this matrix P and the coefficient field c evaluate the value for c^T P c 
	   Specialization for double valued vectors 
	   \param coefficients B-Spline coefficient field c 
	   \param[out] gradient gradient of the divcurl cost 
	   \returns <c^T, P, c>
	*/
	double evaluate(const T2DDatafield<C2DDVector>& coefficients, CDoubleVector& gradient) const; 

	/**
	   \returns fild size this matrix was created for 
	 */
	const C2DBounds& get_size() const; 

	/**
	   re-initialize the matrix (if the values are changed)
	 */
	void reset(const C2DBounds& size, const C2DFVector& range, const CSplineKernel& kernel, 
		   double wd, double wr); 
private: 
	class C2DPPDivcurlMatrixImpl *impl; 

}; 
NS_MIA_END

#endif
