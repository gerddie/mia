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

#ifndef mia_2d_model_hh
#define mia_2d_model_hh

#include <mia/core/typedescr.hh>
#include <mia/core/regmodel.hh>

#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   @cond INTERNAL 
   @ingroup registration 
*/
template <> 
struct RegistrationTraits<2> {
	typedef C2DImage Data;
	typedef C2DFVectorfield Force; 
	typedef C2DFVectorfield Transformation; 
}; 
/// @endcond 

/**
   \ingroup registration 
   \brief the 2D time-marching registration model 
   
   The 2D time-marching registration model 
*/
typedef TRegModel<2> C2DRegModel;

/// Pointer type for the registration model 
typedef std::shared_ptr<C2DRegModel > P2DRegModel;

/**
   @ingroup registration 
   @brief Plugin for the 2D registration model. 
*/
typedef TFactory<C2DRegModel>  C2DRegModelPlugin;

/**
   @ingroup registration 
   @brief Plugin handler for the 2D registration model. 
*/

template <> const char *  const TPluginHandler<C2DRegModelPlugin>::m_help; 

typedef THandlerSingleton< TFactoryPluginHandler<C2DRegModelPlugin> > C2DRegModelPluginHandler;

extern template class EXPORT_2D TRegModel<2>;
extern template class EXPORT_2D TPlugin<C2DImage, regmodel_type>;
extern template class EXPORT_2D TFactory<C2DRegModel>;
extern template class EXPORT_2D TFactoryPluginHandler<C2DRegModelPlugin>;
extern template class EXPORT_2D THandlerSingleton< TFactoryPluginHandler<C2DRegModelPlugin> >; 


NS_MIA_END

#endif

