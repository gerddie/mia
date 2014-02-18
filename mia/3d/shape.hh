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
#ifndef mia_3d_shape_hh
#define mia_3d_shape_hh

#include <mia/core/shape.hh>
#include <mia/core/factory.hh>
#include <mia/3d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup misc 
   @brief a 3D bit shape for morphological processing \sa TShape
*/
typedef TShape<T3DVector, C3DBitImage> C3DShape;

/**
   @ingroup misc 
   @brief pointer to a 3D bit shape for morphological processing \sa TShape
*/
typedef std::shared_ptr<C3DShape > P3DShape;

/**
   @ingroup misc 
   @brief Base class for the 3D shape plug-ins 
*/
typedef TFactory<C3DShape> C3DShapePlugin;

/**
   @ingroup misc 
   @brief Plugin handler for 3D shapes 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C3DShapePlugin> > C3DShapePluginHandler;


/** 
    @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
class EXPORT_3D C3DShapePluginHandlerTestPath {
public: 
	C3DShapePluginHandlerTestPath(); 
}; 
/// @endcond 


/// @cond never
FACTORY_TRAIT(C3DShapePluginHandler); 
/// @endcond 

NS_MIA_END

#endif

