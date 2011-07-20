/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/export_handler.hh>

#include <mia/2d/transformfactory.hh>
#include <mia/internal/transformfactory.cxx>

NS_MIA_BEGIN

template class TTransformCreator<C2DTransformation>; 
template class TTransformCreatorPlugin<C2DTransformation>; 

C2DTransformCreatorHandlerTestPath::C2DTransformCreatorHandlerTestPath()
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path(MIA_BUILD_ROOT"/mia/2d/transform"));
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

EXPLICIT_INSTANCE_DERIVED_FACTORY_HANDLER(C2DTransformCreator, C2DTransformCreatorPlugin); 


NS_MIA_END
