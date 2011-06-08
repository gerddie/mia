/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_2d_trait_hh
#define mia_2d_trait_hh


#include <mia/internal/dimtrait.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/multicost.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN
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
NS_MIA_END
#endif
