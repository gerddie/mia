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

#ifndef mia_2d_fullcost_hh
#define mia_2d_fullcost_hh

#include <mia/2d/transform.hh>
#include <mia/2d/cost.hh>
#include <mia/core/export_handler.hh>
#include <mia/template/fullcost.hh>

NS_MIA_BEGIN

/**
   \ingroup registration 
   \brief Base class for the full cost functions 
   
   This typedef is the base class of the full-cost cost funtions. 
*/
typedef TFullCost<C2DTransformation> C2DFullCost; 

/// Pointer type for the full-cost function types 
typedef C2DFullCost::Pointer P2DFullCost; 

/// Plug-in type for 2D full cost plug-ins 
typedef TFullCostPlugin<C2DTransformation> C2DFullCostPlugin; 

/// plugin-handler for the full-cost plug-ins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DFullCostPlugin> > C2DFullCostPluginHandler;

/**
   \ingroup convine 
   Function to create a full-cost object from a string description
   \param descr 
   \returns the full-cost object 
*/
inline P2DFullCost produce_2dfullcost(const std::string& descr) 
{
	return C2DFullCostPluginHandler::instance().produce(descr); 
}

/** 
    @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
struct EXPORT_2D C2DFullCostPluginHandlerTestPath {
	C2DFullCostPluginHandlerTestPath(); 
private: 
	C2DImageCostPluginHandlerTestPath cost_path_init; 
};
/// @endcond 


/// @cond NEVER 
FACTORY_TRAIT(C2DFullCostPluginHandler); 
/// @endcond 

NS_MIA_END

#endif
