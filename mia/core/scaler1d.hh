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

#ifndef mia_core_scale1d_hh
#define mia_core_scale1d_hh

#include <cassert>
#include <vector>
#include <memory>

#include <mia/core/vector.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/interpolator1d.hh>
#include <mia/core/spacial_kernel.hh>

NS_MIA_BEGIN

/**
   \ingroup interpol
   \brief A class for general scaling of one-dimensional arrays. 

   Class for general scaling of one-dimensional arrays. kernels are the separable B-spline 
   functions.  For upscaling, an interpolator is created by using the provided interpolator factory.
   Downscaling is done solving an overdetermined linear system of equations Ax = y. At construction 
   time the matrix A is QR decomposed, and later the downscaling is done by solving QRx=y
   \todo see if scaling can be expressed by a filter 
   \todo seems like downscaling isn't done properly 
*/

class EXPORT_CORE C1DScalar {
public:
	typedef std::vector<double> std_double_vector; 

	/**
	    Create the scaler prividing the given interpolator kernel.
	    \param kernel interpolation kernel 
	    \param in_size
	    \param out_size the input will be scaled to the given size, the scale is set accordingly 
	 */

	C1DScalar(const CSplineKernel& kernel, size_t in_size, size_t out_size);

	/**
	    Create the scaler providing the given interpolator kernel.
	    \param kernel interpolation kernel 
	    \param in_size
	    \param scale factor by which tho scale the input size, the interpolation will be 
	    exact with respect to the scale 
	 */

	C1DScalar(const CSplineKernel& kernel, size_t in_size, double scale);

	/**
	   Scaling operator.
	   \param input input data
	   \param[out] output when calling the function, the size of this vector must be set to the requested
	   size. The path for down or upscaling is automatically selected.
	 */
	void operator () (const std_double_vector& input, std_double_vector& output) const;

	/**
	   Alternate interface for scaling. Here the input data has to be copied into 
	   the pre-acclocated memory by using the \a input_begin() and \a input_end() iterators, 
	   and the result can be read using the \a output_begin(), \a output_end() iterators. 
	 */
	void run();

	/// \returns begin iterator of the input buffer 
	std_double_vector::iterator input_begin();  
	/// \returns end iterator of the input buffer 
	std_double_vector::iterator input_end();  

	/// \returns begin iterator of the output buffer 
	std_double_vector::iterator output_begin();  
	
	/// \returns end iterator of the output buffer 
	std_double_vector::iterator output_end();  

	/// \returns the size of the output vector
	size_t get_output_size() const; 
private:
	void initialize(const CSplineKernel& kernel); 
	void upscale(const std_double_vector& input, std_double_vector& output) const; 
	void downscale(const std_double_vector& input, std_double_vector& output) const; 

	enum EStrategy {
		scs_fill_output, 
		scs_upscale, 
		scs_copy, 
		scs_downscale, 
		scs_unknown
	}; 

	size_t m_in_size; 
	size_t m_support; 
	double m_scale; 
	std::vector<double> m_poles; 
	EStrategy m_strategy; 
	PSplineBoundaryCondition m_bc; 

	std_double_vector m_input_buffer; 
	
	size_t m_out_size; 
	std_double_vector m_output_buffer; 
	std::vector<CSplineKernel::VWeight> m_weights; 
	std::vector<CSplineKernel::VIndex> m_indices; 

	P1DSpacialKernel m_gauss; 
};



NS_MIA_END


#endif
