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

#ifndef mia_3d_fullcost_hh
#define mia_3d_fullcost_hh

#include <mia/3d/transform.hh>
#include <mia/core/export_handler.hh>
#include <mia/internal/fullcost.hh>

NS_MIA_BEGIN

/// 3D version of the TFullCost
typedef TFullCost<C3DTransformation> C3DFullCost; 

/// pointer of the 3D version of the TFullCost
typedef C3DFullCost::Pointer P3DFullCost; 

/// plugin base class for C3DFullCost objects 
typedef TFullCostPlugin<C3DTransformation> C3DFullCostPlugin;

/// plugin handler for C3DFullCost object creation 
typedef THandlerSingleton<TFactoryPluginHandler<C3DFullCostPlugin> > C3DFullCostPluginHandler;

/// Trait to make C3DFullCost available for creation by command line parsing 
FACTORY_TRAIT(C3DFullCostPluginHandler); 

NS_MIA_END

#endif
