/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_splineparzenmi_hh
#define mia_core_splineparzenmi_hh

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#include <mia/core/splinekernel.hh>
#include <mia/core/histogram.hh>

NS_MIA_BEGIN

/**
   \ingroup registration
   \brief Implementation of mutual information based on B-splines 

This class implements a spline parzen windows based evaluation of the 
mutual information between two images and its gradient w.r.t. to one image. 

For details see 
 	P. Thévenaz, M. Unser, "Optimization of Mutual Information 
	for Multiresolution Image Registration," IEEE Tran on Img Proc, 
	vol. 9, no. 12, pp. 2083-2099, December 2000. 

*/
class EXPORT_CORE CSplineParzenMI {
public: 
	/**
	   Constructor of the mutual information class 
	   @param rbins number of bins in reference intensity range
	   @param rkernel B-spline kernel for filling and evaluating reference intensities 
	   @param mbins number of bins in moving intensity range
	   @param mkernel B-spline kernel for filling and evaluating moving intensities
	   @param cut_histogram the percentage of the histogram to clamp at the lower and upper 
	   end in order to eliminate outliers [0, 40] 
	*/
	CSplineParzenMI(size_t rbins, PSplineKernel rkernel,
			size_t mbins, PSplineKernel mkernel, double cut_histogram); 
	

	/**
	   Fill the histogram structures and caches
	   @tparam MovIterator forward iterator type for moving image 
	   @tparam RefIterator forward iterator type for reference image 
	   @param mov_begin begin of moving image range 
	   @param mov_end end of moving image range 
	   @param ref_begin begin of reference image range 
	   @param ref_end end of reference image range 
	 */

	template <typename MovIterator, typename RefIterator>
	BOOST_CONCEPT_REQUIRES( ((::boost::ForwardIterator<MovIterator>))
				((::boost::ForwardIterator<RefIterator>)),
				(void)
				)
		fill(MovIterator mov_begin, MovIterator mov_end, 
		     RefIterator ref_begin, RefIterator ref_end); 


	/**
	   Fill the histogram structures and caches by using a mask 
	   @tparam MovIterator forward iterator type for moving image 
	   @tparam RefIterator forward iterator type for reference image 
	   @param mov_begin begin of moving image range 
	   @param mov_end end of moving image range 
	   @param ref_begin begin of reference image range 
	   @param ref_end end of reference image range 
	   @param mask_begin begin of mask image range 
	   @param mask_end end of mask image range 
	   
	 */

	template <typename MovIterator, typename RefIterator, typename MaskIterator>
		void fill(MovIterator mov_begin, MovIterator mov_end, 
			  RefIterator ref_begin, RefIterator ref_end, 
			  MaskIterator mask_begin, MaskIterator mask_end);
	

	/**
	   @returns the value of the mutual information that is maximized if the 
	   given input images are equal. 
	 */
	double value() const; 

	/**
	   Evaluate the gradient of the MI with respect to a given intensity pair using 
	   an approximation  
	   @param moving intensity in the moving image 
	   @param reference intensity in the moving image 
	   @returns gradient 
	 */
	double get_gradient(double moving, double reference) const; 

	/**
	   Evaluate the gradient of the MI with respect to a given intensity pair 
	   @param moving intensity in the moving image 
	   @param reference intensity in the moving image 
	   @returns gradient 
	 */

	double get_gradient_slow(double moving, double reference) const; 
	/**
	   reset the ranges to force a new evaluation
	*/
	void reset(); 
protected: 
	/** these function is for test purpouses only */ 
	void fill_histograms(const std::vector<double>& values, 
				      double rmin, double rmax,
				      double mmin, double mmax); 
private: 
       
	double scale_moving(double x) const; 
	double scale_reference(double x) const; 

	void evaluate_histograms(); 	
	void evaluate_log_cache();  
        
        size_t m_ref_bins;
	PSplineKernel  m_ref_kernel; 
	size_t m_ref_border; 
	size_t m_ref_real_bins; 
        double m_ref_max;
	double m_ref_min;
        double m_ref_scale; 

	size_t m_mov_bins;
	
	PSplineKernel  m_mov_kernel; 
	size_t m_mov_border; 
	size_t m_mov_real_bins; 
        double m_mov_max;
	double m_mov_min;
        double m_mov_scale; 

		
	std::vector<double> m_joined_histogram; 
	std::vector<double> m_ref_histogram; 
	std::vector<double> m_mov_histogram; 

	std::vector<std::vector<double> > m_pdfLogCache; 
	double  m_cut_histogram; 
	
	template <typename Iterator>
	std::pair<double,double> get_reduced_range(Iterator begin, Iterator end)const; 

	template <typename DataIterator, typename MaskIterator>
        std::pair<double,double> 
		get_reduced_range_masked(DataIterator dbegin, 
					 DataIterator dend, 
					 MaskIterator mbegin)const; 
		
};   

template <typename MovIterator, typename RefIterator>
BOOST_CONCEPT_REQUIRES( ((::boost::ForwardIterator<MovIterator>))
			((::boost::ForwardIterator<RefIterator>)),
			(void)
			)
	CSplineParzenMI::fill(MovIterator mov_begin, MovIterator mov_end, 
				       RefIterator ref_begin, RefIterator ref_end)
{
	std::fill(m_joined_histogram.begin(), m_joined_histogram.end(), 0.0); 

	assert(mov_begin != mov_end); 
	assert(ref_begin != ref_end); 

	if (m_mov_max < m_mov_min) {
		// (re)evaluate the ranges 
		auto mov_range = get_reduced_range(mov_begin, mov_end); 
		if (mov_range.second  ==  mov_range.first) 
			throw std::invalid_argument("relevant moving image intensity range is zero"); 
		m_mov_min = mov_range.first; 
		m_mov_max = mov_range.second;
		m_mov_scale = (m_mov_bins - 1) / (m_mov_max - m_mov_min); 
		cvdebug() << "Mov Range = [" << m_mov_min << ", " << m_mov_max << "]\n"; 
	}


	if (m_ref_max < m_ref_min) {
		auto ref_range = get_reduced_range(ref_begin, ref_end); 
		if (ref_range.second  ==  ref_range.first) 
			throw std::invalid_argument("relevant reference image intensity range is zero"); 
		
		m_ref_min = ref_range.first; 
		m_ref_max = ref_range.second; 
		m_ref_scale = (m_ref_bins - 1) / (m_ref_max - m_ref_min); 
		cvdebug() << "Ref Range = [" << m_ref_min << ", " << m_ref_max << "]\n"; 
	}

       
	std::vector<double> mweights(m_mov_kernel->size()); 
        std::vector<double> rweights(m_ref_kernel->size()); 
	
	size_t N = 0;         
	while (ref_begin != ref_end && mov_begin != mov_end) {
                
		const double mov = scale_moving(*mov_begin); 
		const double ref = scale_reference(*ref_begin); 
		
		const int mov_start = m_mov_kernel->get_start_idx_and_value_weights(mov, mweights) + m_mov_border; 
                const int ref_start = m_ref_kernel->get_start_idx_and_value_weights(ref, rweights) + m_ref_border;  
		
                for (size_t r = 0; r < m_ref_kernel->size(); ++r) {
                        auto inbeg = m_joined_histogram.begin() + 
				m_mov_real_bins * (ref_start + r) + mov_start; 
			double rw = rweights[r]; 
                        std::transform(mweights.begin(), mweights.end(), inbeg, inbeg, 
				       [rw](double mw, double jhvalue){ return mw * rw + jhvalue;});
                }
		
                ++N; 
		++mov_begin; 
		++ref_begin; 
	}

	cvdebug() << "CSplineParzenMI::fill: counted " << N << " pixels\n"; 
	// normalize joined histogram 
	const double nscale = 1.0/N; 
	transform(m_joined_histogram.begin(), m_joined_histogram.end(), m_joined_histogram.begin(), 
		  [&nscale](double jhvalue){return jhvalue * nscale;}); 
	
	evaluate_histograms();  
	evaluate_log_cache(); 
}


template <typename MovIterator, typename RefIterator, typename MaskIterator>
void CSplineParzenMI::fill(MovIterator mov_begin, MovIterator mov_end, 
			   RefIterator ref_begin, RefIterator ref_end, 
			   MaskIterator mask_begin, MaskIterator mask_end)
{
	std::fill(m_joined_histogram.begin(), m_joined_histogram.end(), 0.0); 

	assert(mov_begin != mov_end); 
	assert(ref_begin != ref_end); 
	
	// no mask 
	if (mask_begin == mask_end) 
		fill(mov_begin, mov_end, ref_begin, ref_end); 
	
	if (m_mov_max < m_mov_min) {
		// (re)evaluate the ranges 
		
		auto mov_range = get_reduced_range_masked(mov_begin, mov_end, mask_begin); 
		if (mov_range.second  ==  mov_range.first) 
			throw std::invalid_argument("relevant moving image intensity range is zero"); 
		m_mov_min = mov_range.first; 
		m_mov_max = mov_range.second;
		m_mov_scale = (m_mov_bins - 1) / (m_mov_max - m_mov_min); 
		cvdebug() << "Mov Range = [" << m_mov_min << ", " << m_mov_max << "]\n"; 
	}


	if (m_ref_max < m_ref_min) {

		auto ref_range = get_reduced_range_masked(ref_begin, ref_end, mask_begin); 
		if (ref_range.second  ==  ref_range.first) 
			throw std::invalid_argument("relevant reference image intensity range is zero"); 
		
		m_ref_min = ref_range.first; 
		m_ref_max = ref_range.second; 
		m_ref_scale = (m_ref_bins - 1) / (m_ref_max - m_ref_min); 
		cvdebug() << "Ref Range = [" << m_ref_min << ", " << m_ref_max << "]\n"; 
	}

       
	std::vector<double> mweights(m_mov_kernel->size()); 
        std::vector<double> rweights(m_ref_kernel->size()); 
	
	size_t N = 0;         
	while (ref_begin != ref_end && mov_begin != mov_end) {

		if (*mask_begin) {
			const double mov = scale_moving(*mov_begin); 
			const double ref = scale_reference(*ref_begin); 
			
			const int mov_start = m_mov_kernel->get_start_idx_and_value_weights(mov, mweights) + m_mov_border; 
			const int ref_start = m_ref_kernel->get_start_idx_and_value_weights(ref, rweights) + m_ref_border;  
			
			for (size_t r = 0; r < m_ref_kernel->size(); ++r) {
				auto inbeg = m_joined_histogram.begin() + 
					m_mov_real_bins * (ref_start + r) + mov_start; 
				double rw = rweights[r]; 
				std::transform(mweights.begin(), mweights.end(), inbeg, inbeg, 
					       [rw](double mw, double jhvalue){ return mw * rw + jhvalue;});
			}
			
			++N; 
		}
		++mask_begin; 
		++mov_begin; 
		++ref_begin; 
	}

	cvdebug() << "CSplineParzenMI::fill: counted " << N << " pixels\n"; 
	// normalize joined histogram 
	const double nscale = 1.0/N; 
	transform(m_joined_histogram.begin(), m_joined_histogram.end(), m_joined_histogram.begin(), 
		  [&nscale](double jhvalue){return jhvalue * nscale;}); 
	
	evaluate_histograms();  
	evaluate_log_cache(); 
}

template <typename Iterator>
std::pair<double,double> CSplineParzenMI::get_reduced_range(Iterator begin, Iterator end)const
{
        auto range = std::minmax_element(begin, end); 	
	typedef THistogramFeeder<typename Iterator::value_type> Feeder; 
	THistogram<Feeder> h(Feeder(*range.first, *range.second, 4096));
	h.push_range(begin, end); 
	auto reduced_range = h.get_reduced_range(m_cut_histogram); 
	cvinfo() << "CSplineParzenMI: reduce range by "<< m_cut_histogram
		<<"% from [" << *range.first << ", " << *range.second 
		<< "] to [" << reduced_range.first << ", " << reduced_range.second << "]\n"; 
	return std::make_pair(reduced_range.first, reduced_range.second); 
       
}

template <typename DataIterator, typename MaskIterator>
std::pair<double,double> 
CSplineParzenMI::get_reduced_range_masked(DataIterator dbegin, 
					  DataIterator dend, 
					  MaskIterator mbegin)const
{
	auto ib = dbegin; 
	auto im = mbegin; 
	
	while (! *im++ && ib != dend) 
		++ib; 
	
	if (ib == dend) 
		throw std::runtime_error("CSplineParzenMI: empty mask"); 
	
	double range_max = *ib; 
	double range_min = *ib; 
	++ib; ++im; 
	
	while (ib != dend)  {
		if (*im) {
			if (*ib < range_min) 
				range_min = *ib; 
			if (*ib > range_max) 
				range_max = *ib; 
		}
		++ib; 
		++im; 
	}
	
	
	typedef THistogramFeeder<typename DataIterator::value_type> Feeder; 
	THistogram<Feeder> h(Feeder(range_min, range_max, 4096));

	ib = dbegin; 
	im = mbegin; 
	while (ib != dend)  {
		if (*im)
			h.push(*ib); 
		++ib; 
		++im; 
	}

	auto reduced_range = h.get_reduced_range(m_cut_histogram); 
	cvinfo() << "CSplineParzenMI: reduce range by "<< m_cut_histogram
		 << "% from [" << range_min << ", " << range_max
		 << "] to [" << reduced_range.first << ", " << reduced_range.second << "]\n"; 
	return std::make_pair(reduced_range.first, reduced_range.second); 
       
}

NS_MIA_END
#endif
