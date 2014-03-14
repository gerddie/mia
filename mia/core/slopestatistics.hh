/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_core_slopestatistics_hh
#define mia_core_slopestatistics_hh

#include <vector>
#include <memory>
#include <mia/core/defines.hh>

NS_MIA_BEGIN


/**
   \ingroup perf
   @brief Runs some statistics over a one-dimensional curve 
   
   Evaluates some statistics about a one-dimensional mapping [0, 1,..., N-1] -> R.
   This is mostly used for myocardial perfusion analysis.  
 */

class  EXPORT_CORE CSlopeStatistics {
public:
	/**
	   Ths enumerate is used to identify the time position of the  movement */

	enum EEnergyCenterpos {
		ecp_none = 0, /*!< no identifictaion */
		ecp_begin,    /*!< movement only beginning of sequence  */
		ecp_center,   /*!< movement throuout the sequence or only in the middle */
		ecp_end       /*!< movement only end of sequence  */
	}; 

	/**
	   Initialize the statistice with the data of a curve 
	   @param series 
	   @param index colum index of the series 
	 */
	CSlopeStatistics(const std::vector<float>& series, int index);

	/**
	   The copy constructor is not allowed 
	 */
	CSlopeStatistics(const CSlopeStatistics& other) = delete;

	~CSlopeStatistics();

	/// \returns the curve length evaluated discretly, i.e. taking the \f$\Delta\f$ x = 1 into account
	float get_curve_length() const;

	/// \returns the absolute range of the curve - i.e. max(series) - min(series)
	float get_range() const;

	/// \returns the mean of the norms of the positive terms of the FFT of this curve
	float get_mean_frequency() const;

	/// \returns the index of the maximum frequency energy  slot
	int get_max_frequency_slot() const;


	/// \returns the sum of the mean of the norms of the positive terms of the FFT of this curve
	float get_energy() const; 
	
	/// \returns the mean frequency level based on a Wavelet analysis
	float get_mean_frequency_level() const;

	/// \returns the level and the time index of the wavelet conefficient with the largest absolute value 
	std::pair<int, int> get_peak_level_and_time_index() const; 

	/**
	   \returns the maximum value f(x)/x for this slope. This indicates the relation between the absolute value 
	   of the first intensity peak and its position in time. The larger the earlier in time and the higher. 
	 */
	float get_maximum_gradient_from_zero() const; 

	/// \returns the absolute value of the peak wavelet coefficient
	float get_peak_wavelet_coefficient() const; 

	/// \returns the sum of the absolute values of the wavelet coefficient
	float get_wavelet_energy() const; 

	/// \returns index and value for the minimum or the maximum (whichever has a lower index) of the curve
	std::pair<size_t, float>  get_first_peak() const;

	/// \returns index and value for the minimum or the maximum (whichever has a higher index) of the curve
	std::pair<size_t, float>  get_second_peak() const;

	/// \returns index of the global maximum if the curve is shifted to start at zero and raises first
	std::pair<size_t, float>  get_perfusion_high_peak() const;

	/**\param start_movement indicates the first index where a movement in the series starts. 
	    The part after this time point will be ignored in the gradinet evaluation
	    \returns index and value for the maximum gradient of the curve */
	std::pair<size_t, float>  get_gradient_peak(int start_movement) const;


	/**
	   \returns the sums of the absolute coefficient values per wavelet frequency level 
	 */
	const std::vector<float>& get_level_coefficient_sums() const;
	
        /**
	   \returns a vector containing the time point of identification 
	   of main movement 
	 */
	const std::vector<EEnergyCenterpos>& get_level_mean_energy_position() const;


	/**
	   \returns the mean of the movement positions over all frequency levels 
	 */
	CSlopeStatistics::EEnergyCenterpos get_mean_energy_position() const; 

	/**
	   \return mean time position of the first positive slope region 
	   \remark experimental means to identify LV/RV enhancement slopes 
	 */
	float get_positive_time_mean() const; 
	
	/**
	   Evaluate the change in mean from before the indicated center to the mean 
	   after the indicated center. The value at the specified center is ignored. 
	   \param center balance point to evaluate change 
	   \returns difference in means
	 */
	float get_level_change(size_t center) const; 
	
	/**
	   \returns the index of this slope as it was stored 
	 */
	int get_index() const; 
private:
	struct CSlopeStatisticsImpl *impl;
};

/// Pointer type for the CSlopeStatistics class 
typedef std::shared_ptr<CSlopeStatistics > PSlopeStatistics;
NS_MIA_END

#endif // mia_core_slopestatistics_hh
