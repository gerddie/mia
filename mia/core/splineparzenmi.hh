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


#ifndef SplineMutualInformation_h 
#define SplineMutualInformation_h

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#include <mia/core/interpolator.hh>
#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN

/**
This class implements a spline parzen windows based evaluation of the 
mutual information between two images and its gradient w.r.t. to one image. 

For details see 

*/

class CSplineParzenMI{
public: 
	/**
	   Constructor of the mutual information class 
	   @param rbins number of bins in reference intensity range
	   @param rkernel B-spline kernel for filling and evaluating reference intensities 
	   @param mbins number of bins in moving intensity range
	   @param mkernel B-spline kernel for filling and evaluating moving intensities
	*/
	CSplineParzenMI(size_t rbins, PBSplineKernel rkernel,
                                 size_t mbins, PBSplineKernel mkernel); 
	

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
		fill(MovIterator mov_begin, MovIterator mov_end, RefIterator ref_begin, RefIterator ref_end); 


	/**
	   @returns the value of the mutual information that is maximized if the given input images 
	            are equal. 
	 */
	double value() const; 

	/**
	   Evaluate the gradient of the MI with respect to a given intensity pair 
	   @param moving intensity in the moving image 
	   @param reference intensity in the moving image 
	   @returns gradient 
	 */
	double get_gradient(double moving, double reference) const; 
private: 
       
	double scale_moving(double x) const; 
	double scale_reference(double x) const; 

	void evaluate_histograms(); 	
	void evaluate_log_cache();  
        
        size_t _M_ref_bins;
	PBSplineKernel  _M_ref_kernel; 
	size_t _M_ref_border; 
	size_t _M_ref_real_bins; 
        double _M_ref_max;
	double _M_ref_min;
        double _M_ref_scale; 

	size_t _M_mov_bins;
	
	PBSplineKernel  _M_mov_kernel; 
	size_t _M_mov_border; 
	size_t _M_mov_real_bins; 
        double _M_mov_max;
	double _M_mov_min;
        double _M_mov_scale; 
	
	std::vector<double> _M_joined_histogram; 
	std::vector<double> _M_ref_histogram; 
	std::vector<double> _M_mov_histogram; 

	std::vector<std::vector<double> > _M_pdfLogCache; 
};   

template <typename MovIterator, typename RefIterator>
BOOST_CONCEPT_REQUIRES( ((::boost::ForwardIterator<MovIterator>))
			((::boost::ForwardIterator<RefIterator>)),
			(void)
			)
	CSplineParzenMI::fill(MovIterator mov_begin, MovIterator mov_end, 
				       RefIterator ref_begin, RefIterator ref_end)
{
	std::fill(_M_joined_histogram.begin(), _M_joined_histogram.end(), 0.0); 

        auto mov_range = std::minmax_element(mov_begin, mov_end); 
        if (*mov_range.second  ==  *mov_range.first) 
                throw std::invalid_argument("Moving image intensity range is zero"); 
        
        _M_mov_min = *mov_range.first; 
        _M_mov_max = *mov_range.second;
        
        auto ref_range = std::minmax_element(ref_begin, ref_end); 
        if (*ref_range.second  ==  *ref_range.first) 
                throw std::invalid_argument("Reference image intensity range is zero"); 
        
        _M_ref_min = *ref_range.first; 
        _M_ref_max = *ref_range.second; 

	_M_ref_scale = (_M_ref_bins - 1) / (_M_ref_max - _M_ref_min); 
	_M_mov_scale = (_M_mov_bins - 1) / (_M_mov_max - _M_mov_min); 

        cvdebug() << "Mov Range = [" << _M_mov_min << ", " << _M_mov_max << "]\n"; 
        cvdebug() << "Ref Range = [" << _M_ref_min << ", " << _M_ref_max << "]\n"; 
       
	std::vector<double> mweights(_M_mov_kernel->size()); 
        std::vector<double> rweights(_M_ref_kernel->size()); 
	
	size_t N = 0;         
	while (ref_begin != ref_end && mov_begin != mov_end) {
                
		const double mov = scale_moving(*mov_begin); 
		const double ref = scale_reference(*ref_begin); 
		
		const int mov_start = _M_mov_kernel->get_start_idx_and_value_weights(mov, mweights) + _M_mov_border; 
                const int ref_start = _M_ref_kernel->get_start_idx_and_value_weights(ref, rweights) + _M_ref_border;  
		
                for (size_t r = 0; r < _M_ref_kernel->size(); ++r) {
                        auto inbeg = _M_joined_histogram.begin() + 
				_M_mov_real_bins * (ref_start + r) + mov_start; 
                        std::transform(mweights.begin(), mweights.end(), inbeg, inbeg, 
				       boost::lambda::_1 * rweights[r] + boost::lambda::_2); 
                }
		
                ++N; 
		++mov_begin; 
		++ref_begin; 
	}

	cvdebug() << "CSplineParzenMI::fill: counted " << N << " pixels\n"; 
	// normalize joined histogram 
	const double scale = 1.0/N; 
	transform(_M_joined_histogram.begin(), _M_joined_histogram.end(), _M_joined_histogram.begin(), 
		  boost::lambda::_1 * scale); 

	evaluate_histograms();  
	evaluate_log_cache(); 
}


NS_MIA_END
#endif
