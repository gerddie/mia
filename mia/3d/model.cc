/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/3d/model.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

template <> const char *  const 
TPluginHandler<C3DRegModelPlugin>::m_help =  
   "These plug-ins define different models for time-marching non-linear image registration algorithms.";

template class TRegModel<3>;
template class TPlugin<C3DImage, regmodel_type>;
template class TFactory<C3DRegModel>;
template class THandlerSingleton< TFactoryPluginHandler<C3DRegModelPlugin> >;
template class TFactoryPluginHandler<C3DRegModelPlugin>;
template class EXPORT_3D TPluginHandler<C3DRegModelPlugin>;

NS_MIA_END
