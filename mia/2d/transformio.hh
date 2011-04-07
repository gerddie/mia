/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_2d_transformio_hh
#define mia_2d_transformio_hh

#include <mia/core/iohandler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN
typedef TIOPlugin<C2DTransformation> C2DTransformationIO; 
typedef TIOHandlerSingleton< TIOPluginHandler<C2DTransformationIO> > C2DTransformationIOPluginHandler;
typedef C2DTransformationIOPluginHandler::Instance::DataKey C2DTransformationDataKey;
NS_MIA_END



#endif
