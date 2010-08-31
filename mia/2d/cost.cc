/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#include <mia/core/export_handler.hh>

#include <mia/2d/cost.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/cost.cxx>

NS_MIA_BEGIN

template class EXPORT_HANDLER TCost<C2DImage, C2DFVectorfield>;
template class EXPORT_HANDLER TPlugin<C2DImage, cost_type>;
template class EXPORT_HANDLER TFactory<C2DImageCost, C2DImage, cost_type>;
template class EXPORT_HANDLER TFactoryPluginHandler<C2DImageCostPlugin>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C2DImageCostPlugin> >;
template class TPluginHandler<C2DImageCostPlugin>;

NS_MIA_END

