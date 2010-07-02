/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif

#include <mia/3d/cost.hh>
#include <mia/core/cost.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN

template class EXPORT_HANDLER TCost<C3DImage, C3DFVectorfield>;
template class EXPORT_HANDLER TPlugin<C3DImage, cost_type>;
template class EXPORT_HANDLER TFactory<C3DImageCost, C3DImage, cost_type>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C3DImageCostPlugin> >;
template class EXPORT_HANDLER TFactoryPluginHandler<C3DImageCostPlugin>;
template class EXPORT_HANDLER TPluginHandler<C3DImageCostPlugin>;


NS_MIA_END
