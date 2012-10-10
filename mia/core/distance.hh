
/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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



#ifndef mia_core_distance_hh
#define mia_core_distance_hh

#include <vector>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   This function evaluates the 1D distance transform 
   of an input function 
   \param[in,out] r at input the squared values of the function to evaluate the distance to 
   at output it contains the distances 
 */

void distance_transform_inplace(std::vector<float>& r); 

NS_MIA_END

#endif
