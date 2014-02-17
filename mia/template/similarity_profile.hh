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

#ifndef mia_internal_similarity_profile_hh
#define mia_internal_similarity_profile_hh

#include <mia/core/fft1d_r2c.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN
/**
   @ingroup perf 
   \brief A class to evaluate image similarity profiles of image series 
   \tparam dim the dimenstion of the image data to be used 

   This class evaluates the similarity profile of a series of images 
   with respect to a certain cost function and a given reference. 
   It can then be used to evaluate a periodic subset and the 
   highest frequency coefficient. 
*/
template <int dim>
class EXPORT_HANDLER TSimilarityProfile {
public: 
        /// the trait to handle dimension based typedefs 
	typedef dimension_traits<dim> this_dim_trait; 
	
	/// Pointer type for cost measure used to create the similarity profile 
	typedef typename this_dim_trait::PFullCost PFullCost; 

	/// type of the image series to create the similarity profile from 
	typedef typename this_dim_trait::ImageSeries ImageSeries; 

	/// pointer type for the actual images 
	typedef typename this_dim_trait::PImage PImage; 
	/**
	   Contruct and prepare the similarity profile. Throws an std::invalid_argument exception 
	   on failure. 
	   \param cost 
	   \param images
	   \param reference
	 */
	TSimilarityProfile(PFullCost cost, const ImageSeries& images, 
			     size_t reference); 
	
	/// copy constructor 
	TSimilarityProfile(const TSimilarityProfile<dim>& org); 
	
	/// assignment operator 
	TSimilarityProfile<dim>& operator = (const TSimilarityProfile<dim>& org); 

	
	/// \returns the peak frequency coefficent and its index
	float get_peak_frequency() const;

	/// @returns a periodic subset including the first and last image of the series
	std::vector<size_t> get_periodic_subset() const; 
private: 
	size_t m_skip; 
	size_t m_reference; 
	mutable float m_peak_freq;
	mutable bool m_peak_freq_valid; 
	std::vector<CFFT1D_R2C::Real> m_cost_values; 
}; 

NS_MIA_END

#endif
