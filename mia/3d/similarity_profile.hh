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

#ifndef mia_3d_similarity_profile_hh
#define mia_3d_similarity_profile_hh

#include <mia/core/fft1d_r2c.hh>
#include <mia/3d/trait.hh>
#include <mia/template/similarity_profile.hh>

NS_MIA_BEGIN
/**
   @ingroup perf
   @brief Specialization of the series similarity profiles for 3D images
*/
typedef TSimilarityProfile<3> C3DSimilarityProfile;
NS_MIA_END

#endif
