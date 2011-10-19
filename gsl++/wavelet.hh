/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#ifndef gslpp_wavelet_hh
#define gslpp_wavelet_hh

#include <gsl++/gsldefines.hh>
#include <cstdlib>
#include <vector>

namespace gsl {

enum EWaveletType {
	wt_haar, 
	wt_haar_centered, 
	wt_daubechies, 
	wt_daubechies_centered, 
	wt_bspline, 
	wt_bspline_centered, 
	wt_none
}; 

class EXPORT_GSL C1DWavelet {
public:
	C1DWavelet(EWaveletType wt, size_t k); 

	~C1DWavelet(); 

	std::vector<double> backward(const std::vector<double>& x) const; 
	std::vector<double> forward(const std::vector<double>& x) const; 
	std::vector<double> forward(const std::vector<float>& x) const; 

private: 
	struct C1DWaveletImpl *impl; 
}; 

}

#endif
