/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/image.hh>

/*
  This code implements the functions needed to support the normalized gradient field 
  cost function.
  
  for details on this cost function see 
  Eldad Haber and Jan Modersitzki, 
   "Beyond Mutual Information: A Simple and Robust  Alternative" 
    Bildverarbeitung f\"ur die Medizin 2005, 
  Eds: Hans-Peter Meinzer, Heinz Handels, Alexander Horsch and Thomas Tolxdorff, 350--354,
  Springer Berlin Heidelberg, 2005
*/



NS_MIA_BEGIN

/**
   Evaluate an approximate of the noise level - specifically this 
   uses the deviation \f$\sigma\f$ of the intensities 
   @param image 
   @returns noise level 
 */
EXPORT_2D float get_noise_level(const C2DImage& image);


/**
   Evaluate the gradient jump level as described in the paper. 
   @param image 
   @returns jump level
 */
EXPORT_2D float get_jump_level(const C2DImage& image);


/**
   Evaluate the gradient jump level as described in the paper by using 
   a specific noise level 
   @param image 
   @param noise_level 
   @returns jump level
 */
EXPORT_2D float get_jump_level(const C2DImage& image, float noise_level);

/**
   Evaluate the normalized gradient field of an image by using a specific noise level 
   @param image 
   @param noise_level 
   @returns NGF 
 */
EXPORT_2D C2DFVectorfield get_nfg_n(const C2DImage& image,  float noise_level);

/**
   Evaluate the normalized gradient field of an image by using a specific jump level 
   @param image 
   @param jump_level2 square of the jump level 
   @returns NGF 
 */
EXPORT_2D C2DFVectorfield get_nfg_j(const C2DImage& image,  float jump_level2);

/**
   Evaluate the normalized gradient field of an image by estimating the noise 
   using  get_noise_level
   @param image 
   @returns NGF 
 */
EXPORT_2D C2DFVectorfield get_nfg(const C2DImage& image);

NS_MIA_END
