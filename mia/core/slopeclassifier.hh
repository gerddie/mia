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

#include <vector>
#include <mia/core/slopevector.hh>


#ifndef mia_core_classifier_hh
#define mia_core_classifier_hh

NS_MIA_BEGIN

/**
   \ingroup perf
   \brief Curve classifier for heart perfusion data 
   
   Classify a set of curves. This class is mostly usefull for the analysis
   of series of perfusion images that are aquired using free breathing.
*/

class  EXPORT_CORE  CSlopeClassifier {
public:
	/// typedef to define the matrix of curves 
	typedef CSlopeColumns Columns; 


	/**
	   Helper structure to store the correlation of two curves 
	 */
	typedef struct  {
		/// correlation 
		float corr; 
		/// index of first row 
		int row1;
		/// index of second row 
		int row2;
	} SCorrelation;

	/**
	   Initialize the classifier with the given curves and the information whether the means were stripped 
	   @param m matrix of curves 
	   @param mean_stripped
	 */
	CSlopeClassifier(const Columns& m, bool mean_stripped);

	/** copy constructor */
	CSlopeClassifier(const CSlopeClassifier& other);
	
	CSlopeClassifier(); 
	
	/// assignment operator 
	CSlopeClassifier& operator =(const CSlopeClassifier& other);

	~CSlopeClassifier();

	/// @return the index of the periodic curve or -1 if none was found 
	int get_periodic_idx() const;
	
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

	/// \returns the absolute difference between the length ofthe longest and second longest curve in the list
	float get_max_slope_length_diff() const;

	/// \returns the maximum value of the correlation between the curves and the rows for which it occures 
	SCorrelation  max_selfcorrelation() const;

	/// \returns the column index of this slope
	int index() const; 

private:
	struct CSlopeClassifierImpl *impl;
};

/// \returns Pearsons correlation coefficient between two series
float  EXPORT_CORE correlation(const std::vector<float>& a, const std::vector<float>& b);

NS_MIA_END

#endif
