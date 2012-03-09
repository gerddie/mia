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

#ifndef mia_2d_model_hh
#define mia_2d_model_hh

#include <mia/core/typedescr.hh>
#include <mia/core/regmodel.hh>

#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

/**
   @cond INTERNAL 
*/
template <> 
struct RegistrationTraits<2> {
	typedef C2DImage Data;
	typedef C2DFVectorfield Force; 
	typedef C2DFVectorfield Transformation; 
}; 


typedef TRegModel<2> C2DRegModel;
typedef std::shared_ptr<C2DRegModel > P2DRegModel;
typedef TFactory<C2DRegModel>  C2DRegModelPlugin;
typedef THandlerSingleton< TFactoryPluginHandler<C2DRegModelPlugin> > C2DRegModelPluginHandler;

/// @endcond 

NS_MIA_END

#endif

