/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#define VSTREAM_DOMAIN "3dtransformcreator"
#include <mia/core/export_handler.hh>

#include <mia/3d/transformfactory.hh>
#include <mia/template/transformfactory.cxx>

NS_MIA_BEGIN


template <> const char *  const 
TPluginHandler<C3DTransformCreatorPlugin>::m_help =  
   "These plug-ins define creators for 3D transformations.";

template class TTransformCreator<C3DTransformation>; 
template class TTransformCreatorPlugin<C3DTransformation>; 

EXPLICIT_INSTANCE_DERIVED_FACTORY_HANDLER(C3DTransformCreator, C3DTransformCreatorPlugin); 

NS_MIA_END
