/* -*- mia-c++  -*-
 *
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

#ifndef mia_3d_trait_hh
#define mia_3d_trait_hh


#include <mia/internal/dimtrait.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/multicost.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/interpolator.hh>

NS_MIA_BEGIN
template <>
struct dimension_traits<3> {
	typedef C3DTransformation Transformation; 
	typedef C3DTransformation::Pointer PTransformation; 
	typedef C3DBounds Size; 
	typedef C3DImage Image; 
	typedef C3DImage::Pointer PImage; 
	typedef std::vector<PImage> ImageSeries; 
	typedef P3DTransformationFactory PTransformationFactory; 
	typedef C3DFullCostList FullCostList; 
	typedef C3DFullCost::Pointer PFullCost; 
	typedef C3DFilter Filter; 
	typedef P3DFilter PFilter; 
	typedef C3DFilterPluginHandler FilterPluginHandler;
	typedef C3DInterpolatorFactory InterpolatorFactory; 
	
}; 
NS_MIA_END
#endif
