/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_3d_landmarklistio_hh
#define mia_3d_landmarklistio_hh

#include <mia/3d/landmarklist.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>

NS_MIA_BEGIN

/**
   @ingroup io 
   @brief 3D Landmark list IO plugin base 
*/
typedef TIOPlugin<C3DLandmarklist> C3DLandmarklistIOPlugin;

#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif

extern template class EXPORT_3D TIOPlugin<C3DLandmarklist>;
extern template class EXPORT_3D TPlugin<C3DLandmarklist, io_plugin_type>;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/**
   @ingroup io 
   @brief 3D Landmark list IO plugin handler 
*/
typedef THandlerSingleton<TIOPluginHandler<C3DLandmarklistIOPlugin> > C3DLandmarklistIOPluginHandler;

/**
   @ingroup io 
   @brief 3D Landmark list data key to load and store to the CDatapool
*/
typedef C3DLandmarklistIOPluginHandler::Instance::DataKey C3DLandmarklistDataKey;

NS_MIA_END

#endif
