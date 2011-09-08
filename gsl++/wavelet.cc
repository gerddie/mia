
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

#include <stdexcept>
#include <iostream>
#include <gsl++/wavelet.hh>
#include <gsl/gsl_wavelet.h>

using std::invalid_argument; 
using std::vector; 
using std::copy; 

namespace gsl {


class C1DWaveletImpl {
public:
	C1DWaveletImpl(EWaveletType wt, size_t k); 

	~C1DWaveletImpl(); 

	template <typename T> 
	vector<double> forward(const vector<T>& x) const; 
	
	vector<double> backward(const vector<double>& x) const; 

private: 
	gsl_wavelet *m_wavelet;
}; 


C1DWavelet::C1DWavelet(EWaveletType wt, size_t k):
	impl(new C1DWaveletImpl(wt,k))
{
}

C1DWavelet::~C1DWavelet()
{
	delete impl; 
}

std::vector<double> C1DWavelet::backward(const std::vector<double>& x) const
{
	return impl->forward(x); 
}

std::vector<double> C1DWavelet::forward(const std::vector<double>& x) const
{
	return impl->forward(x); 
}

std::vector<double> C1DWavelet::forward(const std::vector<float>& x) const
{
	return impl->forward(x); 
}

C1DWaveletImpl::C1DWaveletImpl(EWaveletType wt, size_t k):
	m_wavelet(NULL)
{
	switch (wt) {
	case wt_haar:         m_wavelet = gsl_wavelet_alloc (gsl_wavelet_haar_centered, k); break; 
	case wt_haar_centered:m_wavelet = gsl_wavelet_alloc (gsl_wavelet_haar_centered, k); break; 
	case wt_daubechies:          m_wavelet = gsl_wavelet_alloc (gsl_wavelet_daubechies, k); break; 
	case wt_daubechies_centered: m_wavelet = gsl_wavelet_alloc (gsl_wavelet_daubechies_centered, k); break; 
	case wt_bspline:         m_wavelet = gsl_wavelet_alloc (gsl_wavelet_bspline_centered, k); break; 
	case wt_bspline_centered:m_wavelet = gsl_wavelet_alloc (gsl_wavelet_bspline_centered, k); break; 
	default:
		throw invalid_argument("C1DWavelet: Unknown wavelet type requested");
	}
	if (!m_wavelet) 
		throw invalid_argument("C1DWavelet: Unsupported member or insufficienr memory");
}


C1DWaveletImpl::~C1DWaveletImpl()
{
	gsl_wavelet_free(m_wavelet); 
}

vector<double> C1DWaveletImpl::backward(const vector<double>& x) const
{
	vector<double> result(x); 

	gsl_wavelet_workspace * ws = gsl_wavelet_workspace_alloc (x.size()); 
	gsl_wavelet_transform_inverse(m_wavelet, &result[0], 1, x.size(), ws);
	gsl_wavelet_workspace_free (ws); 

	return result;
}

template <typename T> 
vector<double> C1DWaveletImpl::forward(const vector<T>& x) const
{
	size_t s = x.size(); 
	size_t s1 = 1; 
	while (s) {
		s /= 2; 
		s1 *=2; 
	}
	
	if (s1 < x.size()) 
		s1 *=2;
		
	vector<double> x_size_to_pow2(s1); 
	copy(x.begin(), x.end(), x_size_to_pow2.begin());

	
	gsl_wavelet_workspace * ws = gsl_wavelet_workspace_alloc (s1); 
	gsl_wavelet_transform_forward (m_wavelet, &x_size_to_pow2[0], 1, s1, ws); 
	gsl_wavelet_workspace_free (ws); 
	return x_size_to_pow2; 
}



}
