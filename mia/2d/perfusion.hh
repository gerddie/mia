/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_2d_perfusion_hh
#define mia_2d_perfusion_hh

#include <vector>
#include <mia/core/dictmap.hh>
#include <mia/core/waveletslopeclassifier.hh>
#include <mia/core/ica.hh>
#include <mia/2d/image.hh>
#include <mia/2d/filter.hh>


NS_MIA_BEGIN

/**
   @ingroup perf 
   @brief A class to run an ICA on a heart perfusion series

   This class provides the tools for ICA based 2D perfusion image series. This class is specifically 
   designed for the analysis of free breathingly aquired myocardial perfusion images. 
*/

class  EXPORT_2DMYOCARD C2DPerfusionAnalysis  {
public: 
	/// Possible bases for LV-RV heart segmentation
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


	/**
	   Set the number of ICA iterations 
	   @param maxiter
	 */
	void set_max_ica_iterations(size_t maxiter); 
	
	~C2DPerfusionAnalysis();
	
	/**
	   Run the ICA analysis - keeps a copy of the image series 
	   \param series image series should contain more images thennumber of requested components 
       \param icatool Generator for the used ICA analyis method
	 */

	bool run(const std::vector<C2DFImage>& series, const CIndepCompAnalysisFactory& icatool);


	/**
	   \returns \a true if a periodic component could be identified in the given series
	 */
	bool has_movement() const; 

	/**
	   Evaluate an image cropping filter. This code is specifically designed to deal 
	   with the segmentantion of the left heart ventricle in short axis heart MRI 
	   The algorithm evaluates the centers of the LV and the RV and uses the distance 
	   between both to estimata a bounding box. 
	   Some heuristics are used to check whether the segmentation makes sense
	   
	   @param scale enlargement scale of the bounding box to create the cropping region 
	   @param[out] crop_start returns the left upper corner of the cropping region that can be used  
	   to adjust segmentations
	   @param approach on what input data to base thesegmentation on 
	   @param save_features if not empty store feature images in files with this prefix 
	   @returns the cropping filter or C2DFilterPlugin::ProductPtr() if the segmentation fails. 
	 */
	P2DFilter get_crop_filter(float scale, C2DBounds& crop_start,
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
	void set_approach(CIndepCompAnalysis::EApproach approach);

	/**
	   \returns the RV peak enhancement IC index of -1 if it could not be identified
	 */
	int get_RV_peak_idx() const __attribute__((deprecated)); 
	
	/**
	   \returns the RV enhancement IC index of -1 if it could not be identified
	 */
	int get_RV_idx() const; 
	
	/**
	   \returns the LV peak enhancement IC index of -1 if it could not be identified
	*/
	int get_LV_peak_idx() const __attribute__((deprecated)); 
	
        /**
	   \returns the LV enhancement IC index of -1 if it could not be identified
	*/
	int get_LV_idx() const; 


	/**
	   Set the minimum mean frequency (in breath per heart beat) that decides whether a slope is 
	   considered to be periodic. 
	   \param min_freq The new minimum frequency, a value < 0.0 disables the test (this is the default) 
	   \remark this values is simply passed through to the wavelet slope classifier 
	 */
	void set_min_movement_frequency(float min_freq); 


        /**
	   \returns the perfusion enhancement IC index of -1 if it could not be identified
	*/
	int get_perfusion_idx() const; 

        /**
	   \returns the perfusion enhancement IC index of -1 if it could not be identified
	*/
	int get_movement_idx() const; 

	
	/**
	   Dictionary for segmentation method flags 
	 */
	static TDictMap<EBoxSegmentation> segmethod_dict; 

	/**
	   Use an experimental model to create a initial guess. 
	 */
	void set_use_guess_model(); 


	/**
	   Save the mixin matrix to a file. 
	   @param coefs_name output file name 
	 */
	void save_coefs(const std::string&  coefs_name)const; 

	/**
	   Save the feature image to some PNG files. 
	   @param base_name output file name base 
	 */
	void save_feature_images(const std::string&  base_name)const; 

	/**
	   @param index of the feature image requested, set negative to request the mean image. 
	   \returns the requested feature image 
	 */
	P2DImage get_feature_image(int index) const; 
	
	/**
	   @returns the LV maximum peak enhancement time index, or -1 if not identified 
	 */
	int get_LV_peak_time() const; 

	/**
	   @returns the RV maximum peak enhancement time index, or -1 if not identified 
	 */
	int get_RV_peak_time() const; 

	/**
	   Obtain the mixing curve of a vertain component
	   @param idx index of the requested mixing series; 
	   @returns the mixing curve
	 */
	std::vector<float> get_mixing_curve(unsigned  idx) const;
	
private: 
	struct C2DPerfusionAnalysisImpl *impl; 

}; 

NS_MIA_END

#endif
