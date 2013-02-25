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

#include <config.h> 

#include <mia/2d/inittesthandlers.hh>


NS_MIA_BEGIN
namespace bfs = ::boost::filesystem;

C2DVFIOPluginHandlerTestPath::C2DVFIOPluginHandlerTestPath()
{
	CPathNameArray kernelsearchpath({bfs::path(MIA_BUILD_ROOT"/mia/2d/io")});
	C2DVFIOPluginHandler::set_search_path(kernelsearchpath);

}

C2DTransformCreatorHandlerTestPath::C2DTransformCreatorHandlerTestPath()
{
	CPathNameArray kernelsearchpath({bfs::path(MIA_BUILD_ROOT"/mia/2d/transform")});
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

C2DTransformationIOPluginHandlerTestPath::C2DTransformationIOPluginHandlerTestPath()
{
	CPathNameArray kernelsearchpath({bfs::path(MIA_BUILD_ROOT"/mia/2d/transio")});
	C2DTransformationIOPluginHandler::set_search_path(kernelsearchpath);
}



NS_MIA_END
