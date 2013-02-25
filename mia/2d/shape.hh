/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#ifndef mia_2d_shape_hh
#define mia_2d_shape_hh

#include <mia/core/shape.hh>
#include <mia/core/factory.hh>
#include <mia/2d/image.hh>

NS_MIA_BEGIN

/// 2D shape class, see also TShape 
typedef TShape<T2DVector, C2DBitImage> C2DShape;

/// Pointer type of the C2DShape 
typedef std::shared_ptr<C2DShape > P2DShape;

/// Base class for Shape generating plug-ins 
typedef TFactory<C2DShape> C2DShapePlugin;

/// Plug-in handler for the shape plug-ins 
typedef THandlerSingleton<TFactoryPluginHandler<C2DShapePlugin> > C2DShapePluginHandler;

/** 
    @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
class EXPORT_2D C2DShapePluginHandlerTestPath {
public: 
	C2DShapePluginHandlerTestPath(); 
}; 
/// @endcond 

/**
   Convenience function to produce a shape from a plugin
   \param descr the description of the shape 
   \returns the newly created shape 
*/

inline P2DShape produce_2d_shape(const std::string& descr) 
{
	return C2DShapePluginHandler::instance().produce(descr); 
}

P2DShape EXPORT_2D rotate_90_degree(const C2DShape& shape); 

/// @cond NEVER 
// Trait to make the shape definition parsable on the command line  
FACTORY_TRAIT(C2DShapePluginHandler); 
/// @endcond 

NS_MIA_END

#endif

