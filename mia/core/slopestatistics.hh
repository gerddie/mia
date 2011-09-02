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


#ifndef mia_core_slopestatistics_hh
#define mia_core_slopestatistics_hh

#include <vector>
#include <memory>
#include <mia/core/defines.hh>

NS_MIA_BEGIN


/**
   \ingroup helpers 
   @brief Runs some statistics over a one-dimensional curve 
   
   Evaluates some statistics about a one-dimensional mapping [0, 1,..., N-1] -> R.
   This is mostly used for perfusion analysis.  
 */

class  EXPORT_CORE CSlopeStatistics {
public:
	enum EEnergyCenterpos {
		ecp_none = 0, 
		ecp_begin, 
		ecp_center, 
		ecp_end
	}; 

	/**
	   Initialize the statistice with the data of a curve 
	   @param series 
	   @param index colum index of the series 
	 */
	CSlopeStatistics(const std::vector<float>& series, int index);

	CSlopeStatistics(const CSlopeStatistics& other) = delete;

	~CSlopeStatistics();

	/// \returns the curve length evaluated discretly, i.e. taking the \f$\Delta\f$ x = 1 into account
	float get_curve_length() const;

	/// \returns the absolute range of the curve - i.e. max(series) - min(series)
	float get_range() const;

	/// \returns the mean of the norms of the positive terms of the FFT of this curve
	float get_mean_frequency() const;

	/// \returns the sum of the mean of the norms of the positive terms of the FFT of this curve
	float get_energy() const; 
	
	/// \returns the mean frequency level based on a Wavelet analysis
	float get_mean_frequency_level() const;

	/// \returns the level and the time index of the wavelet conefficient with the largest absolute value 
	std::pair<int, int> get_peak_level_and_time_index() const; 

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

	const std::vector<float>& get_level_coefficient_sums() const;

	const std::vector<EEnergyCenterpos>& get_level_mean_energy_position() const;
	
	int get_index() const; 
private:
	struct CSlopeStatisticsImpl *impl;
};

/// Pointer type for the CSlopeStatistics class 
typedef std::shared_ptr<CSlopeStatistics > PSlopeStatistics;
NS_MIA_END

#endif // mia_core_slopestatistics_hh
