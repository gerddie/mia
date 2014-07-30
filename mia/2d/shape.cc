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


#include <mia/2d/shape.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/shape.cxx>

NS_MIA_BEGIN


template <>
struct __adjust<T2DVector<int> > {
	static void apply(T2DVector<int>& size, const T2DVector<int>& p)  {
		int x = (p.x < 0 ? -2 * p.x : 2 *  p.x) + 1;
		int y = (p.y < 0 ? -2 * p.y : 2 *  p.y) + 1;

		if (size.x < x)
			size.x = x;

		if (size.y < y)
			size.y = y;

	}
};



template class TShape<T2DVector, C2DBitImage>;

template <> const char *  const 
TPluginHandler<TFactory<C2DShape>>::m_help =  "These plug-ins define 2D structuring elements "
				"to describe neighborhoods for morphological and other filters."; 


P2DShape rotate_90_degree(const C2DShape& shape)
{
	P2DShape result(new C2DShape); 
	
	for (auto p = shape.begin(); p != shape.end(); ++p) {
		C2DShape::value_type r; 

		r.x = p->y; 
		r.y = -p->x; 

		result->insert(r); 
	}
	return result; 
}

EXPLICIT_INSTANCE_HANDLER(C2DShape); 

NS_MIA_END
