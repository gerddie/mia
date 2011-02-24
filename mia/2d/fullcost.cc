/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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

#include <boost/lambda/lambda.hpp>
#include <mia/core/export_handler.hh>
#include <mia/2d/fullcost.hh>

#include <mia/internal/fullcost.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

C2DFullCost::C2DFullCost(double weight):
	TFullCost<C2DTransformation>(weight)
{
}


C2DFullCostPlugin::C2DFullCostPlugin(const char *name):
	TFullCostPlugin<C2DTransformation>(name)
{
}
	
EXPLICIT_INSTANCE_PLUGIN(C2DFullCost);
EXPLICIT_INSTANCE_PLUGIN_HANDLER(C2DFullCostPlugin); 

template class TFullCostPlugin<C2DTransformation>; 
template class TFullCost<C2DTransformation>; 


NS_MIA_END
