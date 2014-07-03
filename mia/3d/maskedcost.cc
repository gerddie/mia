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

#include <mia/3d/maskedcost.hh>
#include <mia/template/masked_cost.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN

template <> const char *  const 
TPluginHandler<TFactory<C3DMaskedImageCost>>::m_help =  
	"3D image similarity kernels evaluate the according similarity measure between "
	"two images by using a mask. These kernels may be used standalone, like e.g. in "
	"linear registration, or will be called from generalized image similarity cost "
	"plug-ins that also take care of transforming and scaling the images during the "
	"image registration process.";

using boost::filesystem::path; 
C3DMaskedImageCostPluginHandlerTestPath::C3DMaskedImageCostPluginHandlerTestPath()
{
	CPathNameArray costsearchpath;
	costsearchpath.push_back( path(MIA_BUILD_ROOT"/mia/3d/maskedcost") );
	C3DMaskedImageCostPluginHandler::set_search_path(costsearchpath);
	
}

template class EXPORT_HANDLER TMaskedCost<C3DImage, C3DBitImage, C3DFVectorfield>;
template class EXPORT_HANDLER TPlugin<C3DImage, masked_cost_type>;
template class EXPORT_HANDLER TFactory<C3DMaskedImageCost>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C3DMaskedImageCostPlugin> >;
template class TFactoryPluginHandler<C3DMaskedImageCostPlugin>;
template class TPluginHandler<C3DMaskedImageCostPlugin>;

NS_MIA_END
