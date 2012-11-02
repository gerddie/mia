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

#ifndef mia_2d_cost_hh
#define mia_2d_cost_hh

#include <mia/core/cost.hh>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/// the base class for simple 2D image cost functions 
typedef TCost<C2DImage, C2DFVectorfield> C2DImageCost;

/// the base class for simple 2D image cost functions plug ins 
typedef TFactory<C2DImageCost> C2DImageCostPlugin;

/// the 2D image cost function plugin handler 
typedef THandlerSingleton<TFactoryPluginHandler<C2DImageCostPlugin> > C2DImageCostPluginHandler;

/// pointer type of simple 2D image cost functions 
typedef std::shared_ptr<C2DImageCost > P2DImageCost;

/** 
    @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
struct EXPORT_2D C2DImageCostPluginHandlerTestPath {
	C2DImageCostPluginHandlerTestPath(); 
}; 
/// @endcond 

/// @cond NEVER 
FACTORY_TRAIT(C2DImageCostPluginHandler);
/// @endcond 

NS_MIA_END

#endif
