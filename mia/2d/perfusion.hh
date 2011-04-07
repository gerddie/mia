/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009-2011
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

#ifndef mia_2d_perfusion_hh
#define mia_2d_perfusion_hh

#include <vector>
#include <mia/core/dictmap.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/2dfilter.hh>

NS_MIA_BEGIN

/**
   This class provides the tools for ICA based 2D perfusion image series. This class is specifically 
   designed for the analysis of free breathingly aquired myocardial perfusion images. 
   
*/

class  EXPORT_2D C2DPerfusionAnalysis  {
public: 
	enum EBoxSegmentation {
		bs_delta_feature, /*!< Segmentation based on the difference of the LV and RV feature images */
		bs_delta_peak,    /*!< Segmentation based on the difference of the LV and RV peak enhancenemt images */
		bs_features,      /*!< Segmentation based on the LV and RV feature images */
		bs_unknown        /*!< place holder */
	}; 
	/**
	   Constructor 
	   \param components number of independend components, 0 = auto estimate from [3,4,5,6,7]
	   \param normalize normalize feature images
	   \param meanstrip strip mean from mixing time curves
	 */
	C2DPerfusionAnalysis(size_t components, bool normalize, 
			     bool meanstrip);


	void set_max_ica_iterations(size_t maxiter); 
	
	~C2DPerfusionAnalysis();
	
	/**
	   Run the ICA analysis - keeps a copy of the image series 
	   \param series image series should contain more images thennumber of requested components 
	 */

	bool run(const std::vector<C2DFImage>& series); 


	/**
	   \returns \a true if a periodic component could be identified in the given series
	 */
	bool has_periodic() const; 

	/**
	   Evaluate an image cropping filter. This code is specifically designed to deal 
	   with the segmentantion of the left heart ventricle in short axis heart MRI 
	   The algorithm evaluates the centers of the LV and the RV and uses the distance 
	   between both to estimata a bounding box. 
	   Some heuristics are used to check whether the segmentation makes sense
	   \param scale enlargement scale of the bounding box to create the cropping region 
	   \retval crop_start returns the left upper corner of the cropping region that can be used  
	   to adjust segmentations
	   \returns the cropping filter or C2DFilterPlugin::ProductPtr() if the segmentation fails. 
	 */
	C2DFilterPlugin::ProductPtr get_crop_filter(float scale, C2DBounds& crop_start,
						    EBoxSegmentation approach, 
						    const std::string& save_features="") const; 


	/**
	   Create uncropped reference images that try to omit the movement component in the image series.  
	*/
	std::vector<C2DFImage> get_references() const; 

	/**
	   Set the ICA seperation approach
	   \param approach FICA_APPROACH_SYMM or FICA_APPROACH_DEFL
	   \todo the parameter should be an enum
	 */
	void set_approach(size_t approach); 

	/**
	   \returns the RV peak enhancement index of -1 if ir could not be identified
	 */
	int get_RV_peak_idx() const; 
	
	/**
	   \returns the LV peak enhancement index of -1 if ir could not be identified
	*/
	int get_LV_peak_idx() const; 

	static TDictMap<EBoxSegmentation> segmethod_dict; 

	void set_use_guess_model(); 

	void save_coefs(const string&  coefs_name)const; 
private: 
	struct C2DPerfusionAnalysisImpl *impl; 

}; 

NS_MIA_END

#endif
