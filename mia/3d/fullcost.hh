/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_3d_fullcost_hh
#define mia_3d_fullcost_hh

#include <mia/3d/transform.hh>
#include <mia/core/export_handler.hh>
#include <mia/template/fullcost.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 
   @brief 3D version of the TFullCost
*/
typedef TFullCost<C3DTransformation> C3DFullCost; 

/**
   @ingroup registration 
   @brief pointer of the 3D version of the TFullCost
*/
typedef C3DFullCost::Pointer P3DFullCost; 

/**
   @ingroup registration 
   @brief plugin base class for C3DFullCost objects 
*/
typedef TFullCostPlugin<C3DTransformation> C3DFullCostPlugin;

/**
   @ingroup registration 
   @brief plugin handler for C3DFullCost object creation 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C3DFullCostPlugin> > C3DFullCostPluginHandler;

/**
   @cond NEVER 
   @ingroup traits 
   @brief  Trait to make C3DFullCost available for creation by command line parsing 
*/
FACTORY_TRAIT(C3DFullCostPluginHandler); 
/// @endcond 


NS_MIA_END

#endif
