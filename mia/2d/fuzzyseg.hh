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

#include <mia/2d/2dimageio.hh>

NS_MIA_BEGIN

struct SFuzzySegParams {
	float residuum; 
	float lambda1; 
	float lambda2;
	SFuzzySegParams(): 
		residuum(0.1), 
		lambda1(2e5), 
		lambda2(2e6)
		{}
}; 

/**
   This function runs a fuzzy c-means segmentation with B-field correction in the input data set. 
   \param[in] src the input image 
   \param[in] noOfClasses number of segmentation classes
   \param[in] residuum limit for optimization 
   \param[out] classes probability images after segmentation 
   \returns the B-field corrected image 
 */
EXPORT_2D P2DImage fuzzy_segment_2d(const C2DImage& src, size_t noOfClasses, const SFuzzySegParams& residuum, 
				    C2DImageVector& classes, P2DImage& gain);

NS_MIA_END


