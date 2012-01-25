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


#ifndef mia_3d_shape_hh
#define mia_3d_shape_hh

#include <mia/core/shape.hh>
#include <mia/core/factory.hh>
#include <mia/3d/3DImage.hh>

NS_MIA_BEGIN

/// a 3D bit shape for morphological processing \sa TShape
typedef TShape<T3DVector, C3DBitImage> C3DShape;

/// pointer to a 3D bit shape for morphological processing \sa TShape
typedef std::shared_ptr<C3DShape > P3DShape;

/// Base class for the 3D shape plug-ins 
typedef TFactory<C3DShape> C3DShapePlugin;

/// Plugin handler for 3D shapes 
typedef THandlerSingleton<TFactoryPluginHandler<C3DShapePlugin> > C3DShapePluginHandler;

/// Trait to make the shape definition parsable on the command line  
FACTORY_TRAIT(C3DShapePluginHandler); 

class EXPORT_3D C3DShapePluginHandlerTestPath {
public: 
	C3DShapePluginHandlerTestPath(); 
}; 


NS_MIA_END

#endif

