/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/3d/imageio.hh>

NS_MIA_BEGIN

/**
   @ingroup filtering 
   @brief Fuzzy c-means segmentation with gain field estimation 
   
   This function runs a fuzzy c-means segmentation with B-field correction in the input data set. 
   \param[in] src the input image 
   \param[in] noOfClasses number of segmentation classes
   \param[in] residuum limit for optimization 
   \param[out] classes probability images after segmentation 
   \returns the B-field corrected image 
 */
EXPORT_3D P3DImage fuzzy_segment_3d(const C3DImage& src, size_t noOfClasses, float residuum, C3DImageVector& classes);

NS_MIA_END


