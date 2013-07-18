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

/* comment (1)
   The problem with spline kernels that have an even support width is that 
   at the right boundary the last weight is always zero, so that it would not be necessary 
   to allocate the extra coefficient, but to make the code easier to read, we allocate 
   an additional space at the end of the array. 
*/
CSplineParzenMI::CSplineParzenMI(size_t rbins, PSplineKernel rkernel,
				 size_t mbins, PSplineKernel mkernel, double cut_histogram):

	m_ref_bins(rbins), 
	m_ref_kernel(rkernel), 
	m_ref_border((rkernel->size() - 1) >> 1), 
	m_ref_real_bins(m_ref_bins + 2 * m_ref_border), 
	m_mov_bins(mbins), 
	m_mov_kernel(mkernel), 
	m_mov_border((mkernel->size() - 1) >> 1), 
	m_mov_real_bins(m_mov_bins + 2 * m_mov_border), 
	m_joined_histogram(m_ref_real_bins * m_mov_real_bins + 1/*(1)*/, 0.0), 
	m_ref_histogram(m_ref_real_bins, 0.0),
	m_mov_histogram(m_mov_real_bins + 1/*(1)*/, 0.0),
	m_pdfLogCache(m_ref_real_bins + 1,vector<double>(m_mov_real_bins + 1 /*(1)*/, 0.0)), 
	m_cut_histogram(cut_histogram)
{
	TRACE_FUNCTION; 
	assert(m_ref_bins > 0); 
	assert(m_mov_bins > 0); 
        reset(); 
}	

void CSplineParzenMI::reset()
{
	// invalidate ranges 
	m_ref_max = -1.0; 
	m_ref_min =  1.0; 
	m_mov_max = -1.0; 
	m_mov_min =  1.0; 
}

void CSplineParzenMI::evaluate_histograms()
{
	TRACE_FUNCTION; 

	// clean reference and moving histogram 
	std::fill(m_mov_histogram.begin(), m_mov_histogram.end(), 0.0); 
	std::fill(m_ref_histogram.begin(), m_ref_histogram.end(), 0.0); 
	
	// evaluate reference and moving histogram from joined histogram 
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
			else {
				*o = 0.0; 
			}
		}
        }
}   

double CSplineParzenMI::scale_moving(double x) const 
{
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
	const double mov_entropy =  entropy(m_mov_histogram.begin(), m_mov_histogram.end() - 1); 
	const double joined_entropy =  entropy(m_joined_histogram.begin(), m_joined_histogram.end() - 1); 
	cvdebug() << "Xhisto: " << m_joined_histogram << "\n"; 
	cvdebug() << "Mhisto: " << m_mov_histogram << "\n"; 
	cvdebug() << "Rhisto: " << m_ref_histogram << "\n"; 

	cvdebug() << "entropies: X:" << joined_entropy  << ", M:" << mov_entropy << "R:" << ref_entropy << "\n"; 

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
	const unsigned int msize =  m_mov_kernel->size(); 

	for ( size_t r= 0; r < m_ref_kernel->size(); ++r ) {
		const double rv_by_et = reference_parzen_values[ r ] * inv_et;
                for ( unsigned int m = 0; m < msize; ++m ){

			const double lc = m_pdfLogCache[ r + start_ref_idx][ m + start_mov_idx ]; 
                        result -= lc * rv_by_et * moving_parzen_derivatives[ m ];
		}
	}
	return result; 
}

double CSplineParzenMI::get_gradient_slow(double moving, double reference) const
{
	TRACE_FUNCTION; 
	double mov = scale_moving(moving); 
	double ref = scale_reference(reference); 

	vector<double> moving_parzen_derivatives(m_mov_kernel->size());
	vector<double> moving_parzen_weights(m_mov_kernel->size());
        vector<double> reference_parzen_values(m_ref_kernel->size()); 
        
        // inverse bin size needed in [1] eqn 24 
        const double inv_et = 1.0 / m_mov_scale;  

	const int start_mov_idx = m_mov_kernel->get_start_idx_and_derivative_weights(mov, moving_parzen_derivatives) 
		+ m_mov_border; 

	m_mov_kernel->get_start_idx_and_value_weights(mov, moving_parzen_weights); 
	const int start_ref_idx = m_ref_kernel->get_start_idx_and_value_weights(ref, reference_parzen_values)
		+ m_ref_border; 
	
	const unsigned int msize =  m_mov_kernel->size(); 

	double sum_pxy = 0.0; 
	double sum_px = 0.0; 
	double sum_dpxy = 0.0; 
	double sum_dpx = 0.0; 
	
	for ( size_t r= 0; r < m_ref_kernel->size(); ++r ) {
		const double wr = reference_parzen_values[ r ]; 
		const auto hjx = m_joined_histogram.begin() + (r + start_ref_idx) * m_mov_real_bins; 
		const double rv_by_et =  wr * inv_et;

                for ( unsigned int mi = 0; mi < msize; ++mi ){
			unsigned int m = start_mov_idx + mi; 
			sum_pxy += wr * moving_parzen_weights[mi] * hjx[m];
			sum_px += wr * moving_parzen_weights[mi] * m_mov_histogram[m]; 
			sum_dpxy += rv_by_et * moving_parzen_derivatives[mi] * hjx[m];
			sum_dpx += rv_by_et * moving_parzen_derivatives[mi] * m_mov_histogram[m]; 
		}
	}
	const double dpxlpx = sum_px > 0.0 ? sum_dpx * (std::log(sum_px)+1) : 0.0; 
	const double dpxylpxy = sum_pxy > 0.0 ? sum_dpxy * (std::log(sum_pxy)+1) : 0.0; 
	return dpxlpx  - dpxylpxy;
}


void CSplineParzenMI::fill_histograms(const std::vector<double>& values, 
				      double rmin, double rmax,
				      double mmin, double mmax)
{
	assert(values.size() == m_ref_bins * m_mov_bins); 

	m_mov_min = mmin; 
	m_mov_max = mmax;
	m_mov_scale = (m_mov_bins -1) / (m_mov_max - m_mov_min); 

	m_ref_min = rmin; 
	m_ref_max = rmax;
	m_ref_scale = (m_ref_bins -1) / (m_ref_max - m_ref_min); 


	std::fill(m_joined_histogram.begin(), m_joined_histogram.end(), 0.0); 
	
	/**
	   Now fill the joined histogram assuming that the input is normalized, but 
	   needs to be distributed properly over the bins. 
	*/
	std::vector<double> mweights(m_mov_kernel->size()); 
        std::vector<double> rweights(m_ref_kernel->size()); 
	const int mov_start = m_mov_kernel->get_start_idx_and_value_weights(0, mweights) + m_mov_border; 
	const int ref_start = m_ref_kernel->get_start_idx_and_value_weights(0, rweights) + m_ref_border;  

	auto iv = values.begin(); 
	auto ijh = m_joined_histogram.begin(); 
	for(unsigned int f = ref_start; f < m_ref_bins + ref_start; ++f, ijh += 2 * m_mov_border) {
		for(unsigned int m = mov_start; m < m_mov_bins + mov_start; ++m, ++iv, ++ijh) {
			for (unsigned int kf = 0; kf <  m_ref_kernel->size(); ++kf) {
				auto rw = rweights[kf]; 
				auto ijh_f = ijh + kf * m_mov_real_bins; 
				const auto v = *iv; 
				transform(mweights.begin(), mweights.end(), ijh_f, ijh_f, 
					  [&rw, &v](double mw, double h){return h + mw * rw * v;}); 
			}
		}
	}

	evaluate_histograms();  
	evaluate_log_cache(); 

}


NS_MIA_END
