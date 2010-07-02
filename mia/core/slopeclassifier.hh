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

#include <vector>
#include <mia/core/defines.hh>


#ifndef mia_core_classifier_hh
#define mia_core_classifier_hh

NS_MIA_BEGIN

/**
   Classify a set of curves. This class is mostly usefull for the analysis
   of series of perfusion images that are aquired usingfree breathing.
*/

class  EXPORT_CORE  CSlopeClassifier {
public:
	typedef std::vector<std::vector<float> > Columns;

	typedef struct  {
		float corr;
		int row1;
		int row2;
	} SCorrelation;

	CSlopeClassifier(const Columns& m, bool mean_stripped=false);
	CSlopeClassifier(const CSlopeClassifier& other);
	CSlopeClassifier(); 
	
	CSlopeClassifier& operator =(const CSlopeClassifier& other);

	~CSlopeClassifier();

	int get_periodic_idx() const;
	int get_RV_idx()const;
	int get_LV_idx() const;
	int get_baseline_idx() const;
	int get_perfusion_idx() const;

	int get_RV_peak()const;
	int get_LV_peak() const;

	/// \returns the absolute difference between the length ofthe longest and second longest curve in the list
	float get_max_slope_length_diff() const;

	/// \returns the maximum value of the correlation between the curves and the corresponding rows
	SCorrelation  max_selfcorrelation()const;

private:
	struct CSlopeClassifierImpl *impl;
};

/// \returns Pearsons correlation cuefficient between two series
float  EXPORT_CORE correlation(const std::vector<float>& a, const std::vector<float>& b);

NS_MIA_END

#endif
