/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#ifndef mia_2d_transformio_hh
#define mia_2d_transformio_hh

#include <mia/core/iohandler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/2d/transform.hh>

/* Specializations of certain templates to create the necesities for the 
   2D transformation IO */ 
   

NS_MIA_BEGIN

/// The base class for 2D transformation IO plugins 
typedef TIOPlugin<C2DTransformation> C2DTransformationIO; 

/// The 2D transformationb plugin handler 
typedef TIOHandlerSingleton< TIOPluginHandler<C2DTransformationIO> > C2DTransformationIOPluginHandler;

/// The data key type for the CDatapool storage 
typedef C2DTransformationIOPluginHandler::Instance::DataKey C2DTransformationDataKey;
NS_MIA_END



#endif
