/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_model_hh
#define mia_3d_model_hh

#include <mia/core/factory.hh>
#include <mia/core/regmodel.hh>
#include <mia/3d/image.hh>


NS_MIA_BEGIN

/**
   @cond INTERNAL 
*/

template <> 
struct RegistrationTraits<3> {
	typedef C3DImage Data;
	typedef C3DFVectorfield Force; 
	typedef C3DFVectorfield Transformation; 
}; 

/// the 3D registration model base class 
typedef TRegModel<3> C3DRegModel;

/// the 3D registration model base class pointer 
typedef std::shared_ptr<C3DRegModel > P3DRegModel;

/// the 3D registration model plug-in base class 
typedef TFactory<C3DRegModel>  C3DRegModelPlugin;

/// the 3D registration model plug-in handler 
typedef THandlerSingleton< TFactoryPluginHandler<C3DRegModelPlugin> > C3DRegModelPluginHandler;

/// @endcond 

NS_MIA_END

#endif
