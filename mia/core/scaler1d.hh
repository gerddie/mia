/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
   Class for scaling of one-dimensional arrays. kernels are the separable B-spline 
   functions 
   for upscaling, an interpolator is created by using the provided interpolator factory.
*/

class EXPORT_CORE C1DScalarFixed {
public:
	/**
	    Create the scaler prividing the given interpolator factory.
	    \param ipf
	    \param in_size
	    \param out_size
	 */

	C1DScalarFixed(const CBSplineKernel& kernel, size_t in_size, size_t out_size);

	/**
	   Scaling operator.
	   \param input input data
	   \retval output when calling the function, the size of this vector mist be set to the requested
	   size. The path for down or upscaling is automatically selected.
	 */
	void operator () (const gsl::DoubleVector& input, gsl::DoubleVector& output) const;
	void run();

	gsl::DoubleVector::iterator input_begin();  
	gsl::DoubleVector::iterator input_end();  

	gsl::DoubleVector::iterator output_begin();  
	gsl::DoubleVector::iterator output_end();  

private:

	void upscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const; 
	void downscale(const gsl::DoubleVector& input, gsl::DoubleVector& output) const; 


	gsl::DoubleVector filter_line(const gsl::DoubleVector& coeff)const; 
	double initial_coeff(const gsl::DoubleVector& coeff, double pole)const; 
	double initial_anti_coeff(const gsl::DoubleVector& coeff, double pole)const;
	
	enum EStrategy {
		scs_fill_output, 
		scs_upscale, 
		scs_copy, 
		scs_downscale, 
		scs_unknown
	}; 

	size_t _M_in_size; 
	size_t _M_out_size; 
	size_t _M_support; 
	std::vector<double> _M_poles; 
	EStrategy _M_strategy; 

	gsl::DoubleVector _M_input_buffer; 
	gsl::DoubleVector _M_output_buffer; 
	std::vector<std::vector<double> > _M_weights; 
	std::vector<std::vector<int> > _M_indices; 
	gsl::Matrix _M_A; 	
	gsl::DoubleVector _M_tau; 
	mutable gsl::DoubleVector _M_residual; 

};



NS_MIA_END


#endif
