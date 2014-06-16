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

#ifndef mia_2d_transformio_hh
#define mia_2d_transformio_hh

#include <mia/core/iohandler.hh>
#include <mia/core/ioplugin.hh>
#include <mia/2d/transform.hh>

/* Specializations of certain templates to create the necesities for the 
   2D transformation IO */ 
   

NS_MIA_BEGIN

/**
   @ingroup io 
   \brief The base class for 2D transformation IO plugins 
*/ 
typedef TIOPlugin<C2DTransformation> C2DTransformationIO; 


/**
   \ingroup io
   \brief The non-singleton plug-in handler for 3D transformations 
*/


class C2DTransformIOPluginHandlerImpl: public TIOPluginHandler<C2DTransformationIO> {
protected:  
	C2DTransformIOPluginHandlerImpl(); 
};

/**
   @ingroup io 
   \brief The 2D transformationb plugin handler 
*/
typedef THandlerSingleton< C2DTransformIOPluginHandlerImpl > C2DTransformationIOPluginHandler;

/**
   @ingroup io 
   \brief The data key type for the CDatapool storage 
*/
typedef C2DTransformationIOPluginHandler::Instance::DataKey C2DTransformationDataKey;


/**
   \ingroup io 
   \brief convenienance function to load a transformation 
   \param file file name to load the transformation from
   \returns the loaded transformation or an empty std::shared_ptr 
*/
inline P2DTransformation load_2dtransform(const std::string& file)
{
	return C2DTransformationIOPluginHandler::instance().load(file); 
}

/**
   \ingroup io 
   \brief specialized version to load a transformation 
   \param file file name to load the transformation from
   \returns the loaded transformation or an empty std::shared_ptr 
 */
template <>
inline P2DTransformation load_transform<P2DTransformation>(const std::string& file)
{
	return load_2dtransform(file); 
}


/**
   \ingroup io 
   \brief convenienance function to save a transformation 
   \param file file name to store the transformation in 
   \param transform the transformation to be stored
*/
inline bool save_transform(const std::string& file, const C2DTransformation& transform)
{
	return C2DTransformationIOPluginHandler::instance().save(file, transform); 
}


NS_MIA_END



#endif
