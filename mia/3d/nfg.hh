/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/3d/image.hh>

/*
  This code implements the functions needed to support the NFG cost function.


*/



NS_MIA_BEGIN

/**
   @ingroup registration 
   @brief evaluate an approximation of the noise level of an image
   @param image the input image 
   @return the noise level 
 */
EXPORT_3D float get_noise_level(const C3DImage& image);

/**
   @ingroup registration 
   @brief evaluate the noise-vs-gradient jump level for the normalized gradient field evaluation 
   @param image the input image 
   @return the jump level 
 */
EXPORT_3D float get_jump_level(const C3DImage& image);

/**
   @ingroup registration 
   @brief evaluate the noise-vs-gradient jump level for the normalized gradient field evaluation 
   @param image the input image 
   @param noise_level 
   @return the jump level 
 */
EXPORT_3D float get_jump_level(const C3DImage& image, float noise_level);

/**
   @ingroup registration 
   @brief evaluate normalized gradiend field of an image 

   Evaluate the normalized gradient field of an image using a pre-defined noise level 
   @param image the input image 
   @param noise_level 
   @return the normalized gradient field 
 */
EXPORT_3D C3DFVectorfield get_nfg_n(const C3DImage& image,  float noise_level);

/**
   @ingroup registration 
   @brief evaluate normalized gradiend field of an image 

   Evaluate the normalized gradient field of an image using a pre-defined squared jump-level 
   @param image the input image 
   @param jump_level2
   @return the normalized gradient field 
 */
EXPORT_3D C3DFVectorfield get_nfg_j(const C3DImage& image,  float jump_level2);

/**
   @ingroup registration 
   @brief evaluate normalized gradiend field of an image 

   Evaluate the normalized gradient field of an image. Noise level and jump level 
   are estimated. 
   @param image the input image 
   @return the normalized gradient field 
 */
EXPORT_3D C3DFVectorfield get_nfg(const C3DImage& image);

NS_MIA_END
