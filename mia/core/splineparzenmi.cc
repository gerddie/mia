/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010
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



#define VSTREAM_DOMAIN "SplineMutualInformation"
#include <boost/algorithm/minmax_element.hpp>


#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/core/splineparzenmi.hh>
#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
using std::string; 
using std::setw; 
using std::vector; 
using std::invalid_argument; 

using boost::lambda::_1;
using boost::lambda::_2;


CSplineParzenMI::CSplineParzenMI(size_t rbins, PBSplineKernel rkernel,
				 size_t mbins, PBSplineKernel mkernel):

	_M_ref_bins(rbins), 
	_M_ref_kernel(rkernel), 
	_M_ref_border((rkernel->size() + 1) >> 1), 
	_M_ref_real_bins(_M_ref_bins + 2 * _M_ref_border), 
	_M_mov_bins(mbins), 
	_M_mov_kernel(mkernel), 
	_M_mov_border((mkernel->size() + 1) >> 1), 
	_M_mov_real_bins(_M_mov_bins + 2 * _M_mov_border), 
	_M_joined_histogram(_M_ref_real_bins * _M_mov_real_bins, 0.0), 
	_M_ref_histogram(_M_ref_real_bins, 0.0),
	_M_mov_histogram(_M_mov_real_bins, 0.0),
	_M_pdfLogCache(_M_ref_real_bins,vector<double>(_M_mov_real_bins, 0.0))
{
	TRACE_FUNCTION; 
	assert(_M_ref_bins > 0); 
	assert(_M_mov_bins > 0); 
        
}	

void CSplineParzenMI::evaluate_histograms()
{
	TRACE_FUNCTION; 
	// evaluate reference and moving histogram 
	auto jhi = _M_joined_histogram.begin(); 
	auto rhi = _M_ref_histogram.begin(); 
	for(size_t r = 0; r < _M_ref_real_bins; ++r, ++rhi) {
		auto mhi = _M_mov_histogram.begin(); 
		for(size_t m = 0; m < _M_mov_real_bins; ++m, ++jhi, ++mhi) {
			*rhi += *jhi; 
			*mhi += *jhi; 
		}
	}
}

void CSplineParzenMI::evaluate_log_cache()
{
	TRACE_FUNCTION; 
	// evaluate log(p(m,r)/p(m)) cache 
        // evaluate the log ratios and cache them  
        // See [1] eqn. 23, last term 
	auto jhi = _M_joined_histogram.begin(); 
        for (size_t r = 0; r < _M_ref_real_bins; ++r) {
                auto o = _M_pdfLogCache[r].begin(); 
                auto imh = _M_mov_histogram.begin();
                for(size_t m = 0; m < _M_mov_real_bins; ++m, ++jhi, ++o, ++imh) {
                        // what is actaully happening to the gradient 
                        // if p(m,r) != 0, but p(m) = 0? 
                        if ( *imh > 1e-32 &&  *jhi > 1e-32)
                                *o = std::log(*jhi / *imh); 
		}
        }
}   

double CSplineParzenMI::scale_moving(double x) const 
{
        // technically, this clamping of values should not be necessary, 
        // 
        if (x > _M_mov_max) 
                x = _M_mov_max; 
        else if (x < _M_mov_min)
                x = _M_mov_min;
	return (x - _M_mov_min) * _M_mov_scale;
}

double CSplineParzenMI::scale_reference(double x) const 
{

        if (x > _M_ref_max) 
                x = _M_ref_max; 
        else if (x < _M_ref_min)
                x = _M_ref_min;
	return (x - _M_ref_min) * _M_ref_scale;
}

template <typename I> 
double entropy(I begin, I end) 
{
	double result = 0.0; 
	while (begin != end) {
		if (*begin > 0)
			result += *begin * std::log(*begin); 
                ++begin;
	}
	return -result; 
}

	
double CSplineParzenMI::value() const 
{
	TRACE_FUNCTION; 
	const double ref_entropy =  entropy(_M_ref_histogram.begin(), _M_ref_histogram.end()); 
	const double mov_entropy =  entropy(_M_mov_histogram.begin(), _M_mov_histogram.end()); 
	const double joined_entropy =  entropy(_M_joined_histogram.begin(), _M_joined_histogram.end()); 
        cvdebug() << "Entropy:" << joined_entropy  << " - " 
                  << mov_entropy << " - " << ref_entropy << "\n"; 
        return joined_entropy - mov_entropy - ref_entropy; 
}



double CSplineParzenMI::get_gradient(double moving, double reference) const
{
	TRACE_FUNCTION; 
	double mov = scale_moving(moving); 
	double ref = scale_reference(reference); 

	vector<double> moving_parzen_derivatives(_M_mov_kernel->size());
        vector<double> reference_parzen_values(_M_ref_kernel->size()); 
        
        // inverse bin size needed in [1] eqn 24 
        const double inv_et = 1.0 / _M_mov_scale;  

	int start_mov_idx = _M_mov_kernel->get_start_idx_and_derivative_weights(mov, moving_parzen_derivatives) 
		+ _M_mov_border; 
	int start_ref_idx = _M_ref_kernel->get_start_idx_and_value_weights(ref, reference_parzen_values)
		+ _M_ref_border; 
	
	double result = 0.0;
	for ( size_t r= 0; r < _M_ref_kernel->size(); ++r ) {
		const double rv_by_et = reference_parzen_values[ r ] * inv_et;
                for ( size_t m = 0; m < _M_mov_kernel->size(); ++m ){
                        result -= _M_pdfLogCache[ r + start_ref_idx][ m + start_mov_idx ]
				* rv_by_et * moving_parzen_derivatives[ m ];
		}
	}
	return result; 
}

NS_MIA_END
