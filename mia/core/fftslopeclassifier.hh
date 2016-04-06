/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <vector>
#include <mia/core/defines.hh>


#ifndef mia_core_fftslopeclassifier_hh
#define mia_core_fftslopeclassifier_hh

NS_MIA_BEGIN

/**
   \ingroup perf
   \brief Curve classifier for heart perfusion data 
   
   Classify a set of curves. This class is mostly usefull for the analysis
   of series of perfusion images that are aquired using free breathing.
*/

class  EXPORT_CORE  CFFTSlopeClassifier {
public:
	/// typedef to define the matrix of curves 
	typedef std::vector<std::vector<float> > Columns;


	/**
	   Initialize the classifier with the given curves and the information whether the means were stripped 
	   @param m matrix of curves 
	   @param breath_rate approximate frames per breathing cycle
	   @param mean_stripped
	 */
	CFFTSlopeClassifier(const Columns& m, float breath_rate, bool mean_stripped=false);

	/** copy constructor */
	CFFTSlopeClassifier(const CFFTSlopeClassifier& other);
	
	CFFTSlopeClassifier(); 
	
	/// assignment operator 
	CFFTSlopeClassifier& operator =(const CFFTSlopeClassifier& other);

	~CFFTSlopeClassifier();

	/// @returns the indices of periodic curves or an empty vector if none was found 
	std::vector<int> get_periodic_indices() const;
	
	/// @returns all the indices that are not considered to be periodic curves
	std::vector<int> get_mixing_indices() const;
	
	/// @return the index of the RV enhancement curve or -1 if not identified 
	int get_RV_idx()const;
	
	/// @return the index of the LV enhancement curve or -1 if not identified 
	int get_LV_idx() const;

	/// @return the index of the baseline curve or -1 if not identified 
	int get_baseline_idx() const;
	
	/// @return the index of the perfusion curve or -1 if not identified 
	int get_perfusion_idx() const;

	/// @return the time index of the RV peak image or  -1 if not identified 
	int get_RV_peak()const;
	
	/// @return the time index of the LV peak image or  -1 if not identified 
	int get_LV_peak() const;

	/// \returns the absolute difference between the length of the longest and second longest curve in the list
	float get_max_slope_length_diff() const;
private:
	struct CFFTSlopeClassifierImpl *impl;
};

NS_MIA_END

#endif
