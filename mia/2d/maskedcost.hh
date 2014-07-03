/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_2d_masked_cost_hh
#define mia_2d_masked_cost_hh

#include <mia/template/masked_cost.hh>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 
   @brief the image-to-image cost function base class 
*/
typedef TMaskedCost<C2DImage, C2DBitImage, C2DFVectorfield> C2DMaskedImageCost;

/**
   @ingroup registration 
   @brief pointer type of the image-to-image cost function base class 
*/
typedef std::shared_ptr<C2DMaskedImageCost > P2DMaskedImageCost;

/**
   @ingroup registration 
   @brief plug-in for the image-to-image cost function base class 
*/
typedef TFactory<C2DMaskedImageCost> C2DMaskedImageCostPlugin;

/**
   @ingroup registration 
   @brief plug-in handler for the image-to-image cost function base class 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C2DMaskedImageCostPlugin> > C2DMaskedImageCostPluginHandler;

/** 
    @cond INTERNAL
    @ingroup test 
    @brief Initializer for testing un-installed plug-ins 
 */
struct EXPORT_2D C2DMaskedImageCostPluginHandlerTestPath {
	C2DMaskedImageCostPluginHandlerTestPath(); 
}; 
/// @endcond 

/// @cond NEVER 
FACTORY_TRAIT(C2DMaskedImageCostPluginHandler);
/// @endcond 

NS_MIA_END


#endif 
