/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
#include <mia/internal/fullcost.hh>

NS_MIA_BEGIN

typedef TFullCost<C2DTransformation> C2DFullCost; 
typedef C2DFullCost::Pointer P2DFullCost; 

typedef TFullCostPlugin<C2DTransformation> C2DFullCostPlugin; 
typedef THandlerSingleton<TFactoryPluginHandler<C2DFullCostPlugin> > C2DFullCostPluginHandler;

struct EXPORT_2D C2DFullCostPluginHandlerTestPath {
	C2DFullCostPluginHandlerTestPath(); 
private: 
	C2DImageCostPluginHandlerTestPath cost_path_init; 
};

inline P2DFullCost produce_2dfullcost(const std::string& descr) 
{
	return C2DFullCostPluginHandler::instance().produce(descr); 
}


FACTORY_TRAIT(C2DFullCostPluginHandler); 

NS_MIA_END

#endif
