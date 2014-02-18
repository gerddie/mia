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


/**
   \ingroup io 
   \brief convenienance function to load a transformation 
   \param file file name to load the transformation from
   \returns the loaded transformation or an empty std::shared_ptr 
*/
inline P3DTransformation load_3dtransform(const std::string& file)
{
	return C3DTransformationIOPluginHandler::instance().load(file); 
}

/**
   \ingroup io 
   \brief specialized version to load a transformation 
   \param file file name to load the transformation from
   \returns the loaded transformation or an empty std::shared_ptr 
 */
template <>
inline P3DTransformation load_transform<P3DTransformation>(const std::string& file)
{
	return load_3dtransform(file); 
}


/**
   \ingroup io 
   \brief convenienance function to save a transformation 
   \param file file name to store the transformation in 
   \param transform the transformation to be stored
*/
inline bool save_transform(const std::string& file, const C3DTransformation& transform)
{
	return C3DTransformationIOPluginHandler::instance().save(file, transform); 
}


NS_MIA_END



#endif
