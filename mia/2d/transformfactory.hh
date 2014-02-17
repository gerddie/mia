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

#ifndef mia_2d_transformfactory_hh
#define mia_2d_transformfactory_hh

#include <set>
#include <mia/2d/transform.hh>
#include <mia/core/export_handler.hh>
#include <mia/template/transformfactory.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN

/** 
    @ingroup registration
    \brief The base class for 2D transformation creators 
    \remark should the name be C2DTransformFactory? 
*/ 
typedef TTransformCreator<C2DTransformation>  C2DTransformCreator; 

/** 
    @ingroup registration
    \brief Pointer type to the transformation factory
*/
typedef std::shared_ptr<C2DTransformCreator > P2DTransformationFactory;

typedef TTransformCreatorPlugin<C2DTransformation>  C2DTransformCreatorPlugin; 

/** 
    @ingroup registration
    \brief The plugin handler to manage the transformation creators
*/
typedef THandlerSingleton<TFactoryPluginHandler<C2DTransformCreatorPlugin> > C2DTransformCreatorHandler;


/**
   Produce a transformation creator from a string 
   \param descr description of the transformation type
   \returns the creator for the transformation type 
 */
inline P2DTransformationFactory produce_2dtransform_factory(const std::string& descr) 
{
	return C2DTransformCreatorHandler::instance().produce(descr); 
}

/// @cond NEVER  
FACTORY_TRAIT(C2DTransformCreatorHandler); 
/// @endcond 
NS_MIA_END


#endif
