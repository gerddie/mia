/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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

#ifndef mia_3d_fullcost_hh
#define mia_3d_fullcost_hh

#include <mia/3d/transform.hh>
#include <mia/internal/fullcost.hh>

NS_MIA_BEGIN

/**
   Base class for a general cost function. 
 */



class EXPORT_3D C3DFullCost : public TFullCost<C3DTransformation> {
public: 
	
	C3DFullCost(double weight);
}; 

typedef C3DFullCost::Pointer P3DFullCost;


class EXPORT_3D C3DFullCostPlugin: public TFullCostPlugin<C3DTransformation> {
public:
	C3DFullCostPlugin(const char *name);
}; 

typedef THandlerSingleton<TFactoryPluginHandler<C3DFullCostPlugin> > C3DFullCostPluginHandler;
FACTORY_TRAIT(C3DFullCostPluginHandler); 

NS_MIA_END

#endif
