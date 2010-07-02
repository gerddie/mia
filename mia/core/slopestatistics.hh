/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#ifndef mia_core_slopestatistics_hh
#define mia_core_slopestatistics_hh

#include <vector>
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>

NS_MIA_BEGIN


/**
   Evaluates some statistics about a one-dimensional mapping [0, 1,..., N-1] -> R.
 */

class CSlopeStatistics {
public:
	CSlopeStatistics(const std::vector<float>& series);
	~CSlopeStatistics();

	/// \returns the curve length evaluated discretly, i.e. taking the \Delta x = 1 into account
	float get_curve_length() const;

	/// \returns the absolute range of the curve - i.e. max(series) - min(series)
	float get_range() const;

	/// \returns the mean of the norms of the positive terms of the FFT of this curve
	float get_mean_frequency() const;

	/// \returns index and value for the minimum or the maximum (whichever has a lower index) of the curve
	std::pair<size_t, float>  get_first_peak() const;

	/// \returns index and value for the minimum or the maximum (whichever has a higher index) of the curve
	std::pair<size_t, float>  get_second_peak() const;

	/// \returns index of the global maximum if the curve is shifted to start at zero and raises first
	std::pair<size_t, float>  get_perfusion_high_peak() const;
private:
	struct CSlopeStatisticsImpl *impl;
};

typedef SHARED_PTR(CSlopeStatistics) PSlopeStatistics;
NS_MIA_END

#endif // mia_core_slopestatistics_hh
