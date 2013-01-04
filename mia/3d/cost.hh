/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_cost_hh
#define mia_3d_cost_hh

#include <mia/core/cost.hh>
#include <mia/3d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 
   @brief the image-to-image cost function base class 
*/
typedef TCost<C3DImage, C3DFVectorfield> C3DImageCost;

/**
   @ingroup registration 
   @brief pointer type of the image-to-image cost function base class 
*/
typedef std::shared_ptr<C3DImageCost > P3DImageCost;

/**
   @ingroup registration 
   @brief plug-in for the image-to-image cost function base class 
*/
typedef TFactory<C3DImageCost> C3DImageCostPlugin;

/**
   @ingroup registration 
   @brief plug-in handler for the image-to-image cost function base class 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C3DImageCostPlugin> > C3DImageCostPluginHandler;

/** 
    @cond INTERNAL
    @ingroup test 
    @brief Initializer for testing un-installed plug-ins 
 */
struct EXPORT_3D C3DImageCostPluginHandlerTestPath {
	C3DImageCostPluginHandlerTestPath(); 
}; 
/// @endcond 

/// @cond NEVER 
FACTORY_TRAIT(C3DImageCostPluginHandler);
/// @endcond 

NS_MIA_END

#endif
