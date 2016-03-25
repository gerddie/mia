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

#include <mia/core/export_handler.hh>

#include <mia/2d/cost.hh>
#include <mia/core/cost.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>




NS_MIA_BEGIN
namespace bfs=::boost::filesystem; 

template class TCost<C2DImage, C2DFVectorfield>;

template <> const char *  const
TPluginHandler<TFactory<C2DImageCost>>::m_help =  
	"2D image similarity kernels evaluate the according similarity measure between "
	"two images. These kernels may be used standalone, like e.g. in linear registration, "
	"or will be called from generalized image similarity cost plug-ins that also take "
	"care of transforming and scaling the images during the image registration process. ";

EXPLICIT_INSTANCE_HANDLER(C2DImageCost);

NS_MIA_END



