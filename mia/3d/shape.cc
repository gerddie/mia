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

/*
  LatexBeginPluginSection{3D neighborhood shapes}
  \label{sec:3dshapes}
  
  These 3D shapes define neighbourhoods for a variety of image processing filters, 
  like, e.g., morphological filters \ref{filter3d:morph}. 
  Shapes are always binary. 
  
  LatexEnd
*/


#include <mia/core/export_handler.hh>


#ifdef WIN32
#  define EXPORT_SHAPE   __declspec(dllexport)
#else
#  ifdef __GNUC__
#     define EXPORT_SHAPE __attribute__((visibility("default")))
#  else
#     define EXPORT_SHAPE 
#  endif 
#endif


#include <mia/3d/shape.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/shape.cxx>

NS_MIA_BEGIN


template <>
struct __adjust<T3DVector<int> > {
	static void apply(T3DVector<int>& size, const T3DVector<int>& p)  {
		int x = (p.x < 0 ? -2 * p.x : 2 *  p.x) + 1;
		int y = (p.y < 0 ? -2 * p.y : 2 *  p.y) + 1;
		int z = (p.z < 0 ? -2 * p.z : 2 *  p.z) + 1;

		if (size.x < x)
			size.x = x;

		if (size.y < y)
			size.y = y;

		if (size.z < z)
			size.z = z;
	}
};

template class TPlugin<C3DImage, shape_type>;
template class TFactory<C3DShape>;
template class THandlerSingleton<TFactoryPluginHandler<C3DShapePlugin> >;
template class TFactoryPluginHandler<C3DShapePlugin>;
template class TPluginHandler<C3DShapePlugin>;
template class TShape<T3DVector, C3DBitImage>;

using boost::filesystem::path; 
C3DShapePluginHandlerTestPath::C3DShapePluginHandlerTestPath()
{
	list< path> sksearchpath; 
	sksearchpath.push_back( path(MIA_BUILD_ROOT"/mia/3d/shapes"));
	C3DShapePluginHandler::set_search_path(sksearchpath); 
}

NS_MIA_END
