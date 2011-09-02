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

#include <vector>
#include <mia/core/slopevector.hh>


#ifndef mia_core_waveletslopeclassifier_hh
#define mia_core_waveletslopeclassifier_hh

NS_MIA_BEGIN

/**
   \ingroup helpers 
   \brief Curve classifier for heart perfusion data 
   
   Classify a set of curves by using a wavelet analysis. 
   This class is mostly usefull for the analysis of series of perfusion images 
   that are aquired using free breathing.
*/

class  EXPORT_CORE  CWaveletSlopeClassifier {
public:
	/// typedef to define the matrix of curves 
	typedef CSlopeColumns Columns; 

	enum EAnalysisResult {
		wsc_fail, 
		wsc_low_movement, 
		wsc_normal
	}; 
	
        /**
	   Initialize the classifier with the given curves and the information whether the means were stripped 
	   @param m matrix of curves 
	   @param mean_stripped
	 */
	CWaveletSlopeClassifier(const Columns& m, bool mean_stripped);

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

	EAnalysisResult result() const; 
	
private:
	struct CWaveletSlopeClassifierImpl *impl;
};

NS_MIA_END

#endif
