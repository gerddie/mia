/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009-2010
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

#ifndef mia_2d_similarity_profile_hh
#define mia_2d_similarity_profile_hh

#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/fullcost.hh>

NS_MIA_BEGIN
/**
   This class evaluates the similarity profile of a series of images 
   with respect to a certain cost function and a given reference. 
   It can then be used to evaluate a periodic subset and the 
   highest frequency coefficient. 
*/
class EXPORT_2D C2DSimilarityProfile {
public: 
	
	/**
	   Contruct and prepare the similarity profile. Throws an std::invalid_argument exception 
	   on failure. 
	   \param cost 
	   \param images
	   \param reference
	 */
	C2DSimilarityProfile(P2DFullCost cost, const C2DImageSeries& images, 
		     size_t skip, size_t reference); 


	~C2DSimilarityProfile(); 
	
	/// \returns the peak frequency coefficent and its index
	float get_peak_frequency() const;

	/// returns a periodic subset including teh first and last image of the series
	std::vector<size_t> get_periodic_subset() const; 
private: 
	struct C2DSimilarityProfileImpl *impl; 
}; 

NS_MIA_END

#endif
