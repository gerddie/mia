/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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
 *
 */


#ifndef mia_core_scale1d_hh
#define mia_core_scale1d_hh

#include <cassert>
#include <vector>
#include <memory>

#include <gsl++/vector.hh>
#include <gsl++/matrix.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/interpolator1d.hh>

NS_MIA_BEGIN

/**
   \ingroup classes
   \brief A class for general scaling of one-dimensional arrays. 

   Class for general scaling of one-dimensional arrays. kernels are the separable B-spline 
   functions.  For upscaling, an interpolator is created by using the provided interpolator factory.
   Downscaling is done solving an overdetermined linear system of equations Ax = y. At construction 
   time the matrix A is QR decomposed, and later the downscaling is done by solving QRx=y
   \todo see if scaling can be expressed by a filter 
   \todo seems like downscaling isn't done properly 
*/

class EXPORT_CORE C1DScalarFixed {
public:
	/**
	    Create the scaler prividing the given interpolator factory.
	    \param kernel interpolation kernel 
	    \param in_size
	    \param out_size
	 */

	C1DScalarFixed(const CSplineKernel& kernel, size_t in_size, size_t out_size);

	/**
	   Scaling operator.
	   \param input input data
	   \param[out] output when calling the function, the size of this vector must be set to the requested
	   size. The path for down or upscaling is automatically selected.
	 */
	void operator () (const gsl::DoubleVector& input, gsl::DoubleVector& output) const;

	/**
	   Alternate interface for scaling. Here the input data has to be copied into 
	   the pre-acclocated memory by using the \a input_begin() and \a input_end() iterators, 
	   and the result can be read using the \a output_begin(), \a output_end() iterators. 
	 */
	void run();

	/// \returns begin iterator of the input buffer 
	gsl::DoubleVector::iterator input_begin();  
	/// \returns end iterator of the input buffer 
	gsl::DoubleVector::iterator input_end();  

	/// \returns begin iterator of the output buffer 
	gsl::DoubleVector::iterator output_begin();  
	
	/// \returns end iterator of the output buffer 
	gsl::DoubleVector::iterator output_end();  

private:

	void upscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const; 
	void downscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const; 

	enum EStrategy {
		scs_fill_output, 
		scs_upscale, 
		scs_copy, 
		scs_downscale, 
		scs_unknown
	}; 

	size_t m_in_size; 
	size_t m_out_size; 
	size_t m_support; 
	std::vector<double> m_poles; 
	EStrategy m_strategy; 
	CMirrorOnBoundary m_bc; 

	gsl::DoubleVector m_input_buffer; 
	gsl::DoubleVector m_output_buffer; 
	std::vector<std::vector<double> > m_weights; 
	std::vector<std::vector<int> > m_indices; 
	gsl::Matrix m_A; 	
	gsl::DoubleVector m_tau; 
};



NS_MIA_END


#endif
