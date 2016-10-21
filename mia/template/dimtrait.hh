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

#ifndef mia_internal_dimtrait_hh
#define mia_internal_dimtrait_hh

NS_MIA_BEGIN

///   @cond INTERNAL 
struct dimension_traits_placeholder {
	typedef std::shared_ptr<dimension_traits_placeholder> Pointer;
}; 

/**
   \brief Trait used for the non-linear registration 
*/
template <int Dimensions> 
struct dimension_traits {
	typedef dimension_traits_placeholder Transformation; 
	typedef dimension_traits_placeholder PTransformation; 
	typedef dimension_traits_placeholder Size; 
	typedef dimension_traits_placeholder Image; 
	typedef dimension_traits_placeholder PImage; 
	typedef dimension_traits_placeholder ImageSeries; 
	typedef dimension_traits_placeholder PTransformationFactory; 
	typedef dimension_traits_placeholder FullCostList; 
	typedef dimension_traits_placeholder PFullCost; 
	typedef dimension_traits_placeholder Filter; 
	typedef dimension_traits_placeholder FilterPluginHandler;
	typedef dimension_traits_placeholder InterpolatorFactory; 
}; 

template <int Dimensions> 
struct scale_factor_helpers {
	typedef int DIntVector;  

	static DIntVector log2(const DIntVector& x); 
	static DIntVector shift(const DIntVector& x); 
}; 

/**
   \brief Trait used for the watershed algorithms 
*/
template <int Dimensions> 
struct watershed_traits { 
	typedef dimension_traits_placeholder PNeighbourhood; 
	typedef dimension_traits_placeholder Handler; 
	typedef dimension_traits_placeholder FileHandler; 
}; 
/// @endcond 

NS_MIA_END

#endif
