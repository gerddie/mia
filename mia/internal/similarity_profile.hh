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

#ifndef mia_internal_similarity_profile_hh
#define mia_internal_similarity_profile_hh

#include <mia/core/fft1d_r2c.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN
/**
   \brief A class to evaluate image similarity profiles of image series 

   This class evaluates the similarity profile of a series of images 
   with respect to a certain cost function and a given reference. 
   It can then be used to evaluate a periodic subset and the 
   highest frequency coefficient. 
*/
template <int dim>
class EXPORT_HANDLER TSimilarityProfile {
public: 
	typedef dimension_traits<dim> this_dim_trait; 
	typedef typename this_dim_trait::PFullCost PFullCost; 
	typedef typename this_dim_trait::ImageSeries ImageSeries; 
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
