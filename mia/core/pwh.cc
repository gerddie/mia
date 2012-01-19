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


#ifndef mia_core_pwh_hh
#define mia_core_pwh_hh

#include <mia/core/pwh.hh>

#include <cmath>

#include <algorithm>
#include <numeric>
#include <mia/core/interpolator1d.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/fft1d_r2c.hh>
#include <mia/core/errormacro.hh>
#include <complex> 

#include <pwpdf/pwd2.h>

#include <boost/algorithm/minmax_element.hpp>


NS_MIA_BEGIN
using namespace std; 

struct CParzenWindowHistogramImpl {
	CParzenWindowHistogramImpl(double low, double high, 
				   size_t values, const std::vector<double>& samples); 
	~CParzenWindowHistogramImpl(); 
	
	double at(double x) const; 
	double derivative(double x) const; 

	C1DSpacialKernelPlugin::ProductPtr kernel; 
	std::shared_ptr<T1DConvoluteInterpolator<double> > interp; 
	double  range_low; 
	double  range_high;  
	double target_shift; 
	double target_scale; 
	double output_scale; 
}; 

CParzenWindowHistogram::CParzenWindowHistogram(double low, double high, 
					       size_t values, const std::vector<double>& samples):
	impl(new CParzenWindowHistogramImpl(low, high, values, samples))
{
}

CParzenWindowHistogram::~CParzenWindowHistogram()
{
	delete impl; 
}



double CParzenWindowHistogram::operator [] (double i) const
{
	return impl->at(i); 
}

double CParzenWindowHistogram::derivative (double i) const
{
	return impl->derivative(i); 
}



CParzenWindowHistogramImpl::CParzenWindowHistogramImpl(double low, double high, 
						       size_t values, const vector<double>& samples):
	range_low(low), 
	range_high(high), 
	target_shift(low)

{

	ParzenWindowsNfftPDF2 *pwd = parzen_windows_nfft_pdf2_new(samples.size(), values, low, high); 
	vector<double> alpha(samples.size(), 1.0); 
	vector<double> fast_sumresult(values);  
	bool result = parzen_windows_nfft_pdf2_estimate(pwd, &samples[0], &alpha[0], &fast_sumresult[0]); 
	parzen_windows_nfft_pdf2_free(pwd); 
	if (!result) 
		throw invalid_argument("CParzenWindowHistogram: input data bogus (bad range or number of samples"); 

	interp.reset(new T1DConvoluteInterpolator<double>(fast_sumresult, PSplineKernel(new CSplineKernel3()))); 
}

CParzenWindowHistogramImpl::~CParzenWindowHistogramImpl()
{
}

double CParzenWindowHistogramImpl::at(double x) const
{
	// this should be an option of the interpolator 
	if (x >= range_low && x <= range_high) 
		return (*interp)(x); 
	else return 0.0; 
}

double CParzenWindowHistogramImpl::derivative(double x) const
{
	// this should be an option of the interpolator 
	if (x >= range_low && x <= range_high) 
		return interp->derivative_at(x); 
	else return 0.0; 
}
NS_MIA_END

#endif
