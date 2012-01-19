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

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/shape.hh>
#include <mia/internal/dimtrait.hh>

#include <queue>

NS_MIA_BEGIN

template <> 
struct watershed_traits<3> { 
	typedef P3DShape PNeighbourhood; 
	typedef C3DFilterPluginHandler Handler; 
}; 
NS_MIA_END

#include <mia/internal/watershed.hh>

NS_MIA_BEGIN

typedef TWatershedFilterPlugin<3> C3DWatershedFilterPlugin; 
typedef TWatershed<3> C3DWatershed; 


NS_MIA_END
