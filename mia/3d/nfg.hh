/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/3d/3DImage.hh>

/*
  This code implements the functions needed to support the NFG cost function.


*/



NS_MIA_BEGIN

EXPORT_3D float get_noise_level(const C3DImage& image);
EXPORT_3D float get_jump_level(const C3DImage& image);
EXPORT_3D float get_jump_level(const C3DImage& image, float noise_level);
EXPORT_3D C3DFVectorfield get_nfg_n(const C3DImage& image,  float noise_level);
EXPORT_3D C3DFVectorfield get_nfg_j(const C3DImage& image,  float jump_level2);
EXPORT_3D C3DFVectorfield get_nfg(const C3DImage& image);

NS_MIA_END
