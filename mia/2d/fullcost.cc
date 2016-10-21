/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/export_handler.hh>
#include <mia/2d/fullcost.hh>

#include <mia/template/fullcost.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
namespace bfs=::boost::filesystem; 


template class TFullCostPlugin<C2DTransformation>; 
template class TFullCost<C2DTransformation>; 

template <> const char *  const 
TPluginHandler<C2DFullCostPlugin>::m_help =  
	"This class of cost functions evaluates the cost resulting from the "
	"use of a certain transformation. This may include a penalty term that ensures "
	"the smoothness of a transformation or the evaluation of a similarity measure "
	"between a transformed study data set and a reference data set.";


EXPLICIT_INSTANCE_PLUGIN(C2DFullCost);
EXPLICIT_INSTANCE_PLUGIN_HANDLER(C2DFullCostPlugin); 

NS_MIA_END
