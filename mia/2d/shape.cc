/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
  LatexBeginPluginSection{2D neighborhood shapes}
  \label{sec:2dshapes}
  
  These 2D shapes define neighbourhoods for a variety of image processing filters, 
  like, e.g., morphological filters \ref{filter2d:morph} or neighborhood aware threshholding \ref{filter2d:thresh}. 
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


#include <mia/2d/shape.hh>
#include <mia/core/plugin_base.cxx>
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

template <>
struct less_then<T2DVector<int> > {
	bool operator() (const T2DVector<int>& a, const T2DVector<int>& b) const
	{
		return a.x < b.x || (a.x == b.x && a.y < b.y);
	}
};

EXPLICIT_INSTANCE_HANDLER(C2DShape); 
template class TShape<T2DVector, C2DBitImage>;

NS_MIA_END
