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

template <> const char *  const TPluginHandler<C3DRegModelPlugin>::m_help; 

extern template class EXPORT_3D TRegModel<3>; 
extern template class EXPORT_3D TPluginHandler<C3DRegModelPlugin>;
extern template class EXPORT_3D THandlerSingleton< TFactoryPluginHandler<C3DRegModelPlugin> >;
/// the 3D registration model plug-in handler 
typedef THandlerSingleton< TFactoryPluginHandler<C3DRegModelPlugin> > C3DRegModelPluginHandler;

/// @endcond 

NS_MIA_END

#endif
