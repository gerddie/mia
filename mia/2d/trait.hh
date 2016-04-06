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

#ifndef mia_2d_trait_hh
#define mia_2d_trait_hh


#include <mia/template/dimtrait.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/multicost.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN

/**
   @cond INTERNAL 
   \ingroup traits 
   \brief Trait used for the non-linear registration 
*/
template <>
struct dimension_traits<2> {
	typedef C2DTransformation Transformation; 
	typedef C2DTransformation::Pointer PTransformation; 
	typedef C2DBounds Size; 
	typedef C2DImage Image; 
	typedef C2DImage::Pointer PImage; 
	typedef std::vector<PImage> ImageSeries; 
	typedef P2DTransformationFactory PTransformationFactory; 
	typedef C2DFullCostList FullCostList; 
	typedef C2DFullCost::Pointer PFullCost; 
	typedef C2DFilter Filter; 
	typedef P2DFilter PFilter; 
	typedef C2DFilterPluginHandler FilterPluginHandler;
	typedef C2DInterpolatorFactory InterpolatorFactory; 
	
}; 
/// @endcond 
NS_MIA_END
#endif
