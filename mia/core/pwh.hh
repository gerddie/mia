/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __pwh_hh
#define __pwh_hh

#include <vector>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   @cond NOT_EXPOSED 
   
   \ingroup registration 
   \brief This class implements a 1D histogram using Parzen Windows. 

   This class uses NFFT to implement the Parzen windows probability estimation. 
   Input samples are given once with a fixed operation range and the number 
   of output samples used to interpolate the histogram 
   Then the histogram values itself are evaluated using cubic B-Splines. 
   
   @remark Usually this class is not compiled in 
   
 */

class CParzenWindowHistogram {
public: 
	/**
	   Constructor: input samples are clamped to the input range 
	   \param low lower bound of the input sample range 
	   \param high higher bound of the input sample range 
	   \param output_knots number of knodes used for the histogram 
	   \param samples input samples 
	   \todo some kind of shared object should be craated to instanciate only 
	     one fastsum plan that is reused for multiple histograms
	 */
	CParzenWindowHistogram(double low, double high, size_t output_knots, const std::vector<double>& samples); 

	
	~CParzenWindowHistogram(); 

	/// \returns the probability of intensity x 
	double operator [] (double x) const; 

	/// \returns the derivative of probability of intensity x 
	double derivative (double x) const; 
private: 
	struct CParzenWindowHistogramImpl *impl; 
}; 

/// @endcond

NS_MIA_END
#endif
