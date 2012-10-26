/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifndef mia_3d_transformio_hh
#define mia_3d_transformio_hh

#include <mia/core/iohandler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/3d/transform.hh>

NS_MIA_BEGIN

/**
   \ingroup io
   \brief Base class for the generic IO of transformations 
*/
typedef TIOPlugin<C3DTransformation> C3DTransformationIO; 

/**
   \ingroup io
   \brief Plug-in handler for the transformation IO plug-ins 
*/
typedef THandlerSingleton< TIOPluginHandler<C3DTransformationIO> > C3DTransformationIOPluginHandler;

/**
   \ingroup io
   \brief data key type for temporary storage of 3D transformations \sa CDatapool 
*/
typedef C3DTransformationIOPluginHandler::Instance::DataKey C3DTransformationDataKey;
NS_MIA_END



#endif
