/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#define VSTREAM_DOMAIN "SplineMutualInformation"
#include <boost/algorithm/minmax_element.hpp>


#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/core/splineparzenmi.hh>

NS_MIA_BEGIN
using std::string; 
using std::setw; 
using std::vector; 
using std::invalid_argument; 

CSplineParzenMI::CSplineParzenMI(size_t rbins, PSplineKernel rkernel,
				 size_t mbins, PSplineKernel mkernel):

	m_ref_bins(rbins), 
	m_ref_kernel(rkernel), 
	m_ref_border((rkernel->size() + 1) >> 1), 
	m_ref_real_bins(m_ref_bins + 2 * m_ref_border), 
	m_mov_bins(mbins), 
	m_mov_kernel(mkernel), 
	m_mov_border((mkernel->size() + 1) >> 1), 
	m_mov_real_bins(m_mov_bins + 2 * m_mov_border), 
	m_joined_histogram(m_ref_real_bins * m_mov_real_bins, 0.0), 
	m_ref_histogram(m_ref_real_bins, 0.0),
	m_mov_histogram(m_mov_real_bins, 0.0),
	m_pdfLogCache(m_ref_real_bins,vector<double>(m_mov_real_bins, 0.0))
{
	TRACE_FUNCTION; 
	assert(m_ref_bins > 0); 
	assert(m_mov_bins > 0); 
        
}	

void CSplineParzenMI::evaluate_histograms()
{
	TRACE_FUNCTION; 
	// evaluate reference and moving histogram 
	auto jhi = m_joined_histogram.begin(); 
	auto rhi = m_ref_histogram.begin(); 
	for(size_t r = 0; r < m_ref_real_bins; ++r, ++rhi) {
		auto mhi = m_mov_histogram.begin(); 
		for(size_t m = 0; m < m_mov_real_bins; ++m, ++jhi, ++mhi) {
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
	auto jhi = m_joined_histogram.begin(); 
        for (size_t r = 0; r < m_ref_real_bins; ++r) {
                auto o = m_pdfLogCache[r].begin(); 
                auto imh = m_mov_histogram.begin();
                for(size_t m = 0; m < m_mov_real_bins; ++m, ++jhi, ++o, ++imh) {
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
        if (x > m_mov_max) 
                x = m_mov_max; 
        else if (x < m_mov_min)
                x = m_mov_min;
	return (x - m_mov_min) * m_mov_scale;
}

double CSplineParzenMI::scale_reference(double x) const 
{

        if (x > m_ref_max) 
                x = m_ref_max; 
        else if (x < m_ref_min)
                x = m_ref_min;
	return (x - m_ref_min) * m_ref_scale;
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
	const double ref_entropy =  entropy(m_ref_histogram.begin(), m_ref_histogram.end()); 
	const double mov_entropy =  entropy(m_mov_histogram.begin(), m_mov_histogram.end()); 
	const double joined_entropy =  entropy(m_joined_histogram.begin(), m_joined_histogram.end()); 
        cvdebug() << "Entropy:" << joined_entropy  << " - " 
                  << mov_entropy << " - " << ref_entropy << "\n"; 
        return joined_entropy - mov_entropy - ref_entropy; 
}



double CSplineParzenMI::get_gradient(double moving, double reference) const
{
	TRACE_FUNCTION; 
	double mov = scale_moving(moving); 
	double ref = scale_reference(reference); 

	vector<double> moving_parzen_derivatives(m_mov_kernel->size());
        vector<double> reference_parzen_values(m_ref_kernel->size()); 
        
        // inverse bin size needed in [1] eqn 24 
        const double inv_et = 1.0 / m_mov_scale;  

	int start_mov_idx = m_mov_kernel->get_start_idx_and_derivative_weights(mov, moving_parzen_derivatives) 
		+ m_mov_border; 
	int start_ref_idx = m_ref_kernel->get_start_idx_and_value_weights(ref, reference_parzen_values)
		+ m_ref_border; 
	
	double result = 0.0;
	for ( size_t r= 0; r < m_ref_kernel->size(); ++r ) {
		const double rv_by_et = reference_parzen_values[ r ] * inv_et;
                for ( size_t m = 0; m < m_mov_kernel->size(); ++m ){
                        result -= m_pdfLogCache[ r + start_ref_idx][ m + start_mov_idx ]
				* rv_by_et * moving_parzen_derivatives[ m ];
		}
	}
	return result; 
}

NS_MIA_END
