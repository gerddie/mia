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

#include <vector>
#include <mia/core/slopevector.hh>


#ifndef mia_core_waveletslopeclassifier_hh
#define mia_core_waveletslopeclassifier_hh

NS_MIA_BEGIN

/**
   \ingroup perf
   \brief Curve classifier for heart perfusion data 
   
   Classify a set of curves by using a wavelet analysis. 
   This class is mostly usefull for the analysis of series of perfusion images 
   that are aquired using free breathing or with initial breath holding.
*/

class  EXPORT_CORE  CWaveletSlopeClassifier {
public:
	/// typedef to define the matrix of curves 
	typedef CSlopeColumns Columns; 

	/**
	   Enumerate to signal the result of the classification
	 */

	enum EAnalysisResult {
		wsc_fail, /*!< no proper identification of slopes could be achieved */
		wsc_no_movement,  /*!< no movement detected */
		wsc_low_movement, /*!< low level movement detected */
		wsc_normal         /*!< normal movement detected */
	}; 
	
        /**
	   Initialize the classifier with the given curves and the information whether the means were stripped 
	   @param m matrix of curves 
	   @param mean_stripped
	   @param min_freq minimum breathing frequency to consider movement, set to <0 to diable this test 
	 */
	CWaveletSlopeClassifier(const Columns& m, bool mean_stripped, float min_freq);

	/** copy constructor */
	CWaveletSlopeClassifier(const CWaveletSlopeClassifier& other);
	
	CWaveletSlopeClassifier(); 
	
	/// assignment operator 
	CWaveletSlopeClassifier& operator =(const CWaveletSlopeClassifier& other);

	~CWaveletSlopeClassifier();

	/// @return the index of the periodic curve or -1 if none was found 
	int get_movement_idx() const;
	
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
	
	/** @returns  get main breathing movement component energy.  */ 
	float get_movement_indicator() const; 

	/** @returns  get number of movement components.  */ 
	size_t get_number_of_movement_components() const; 

	/** @returns the general result of the identification */
	EAnalysisResult result() const; 

private:
	struct CWaveletSlopeClassifierImpl *impl;
};

NS_MIA_END

#endif
