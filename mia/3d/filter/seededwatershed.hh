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
#include <mia/template/dimtrait.hh>
#include <mia/template/seededwatershed.hh>

NS_MIA_BEGIN

template <> 
struct watershed_traits<3> { 
	typedef P3DShape PNeighbourhood; 
	typedef C3DFilterPluginHandler Handler; 
	typedef C3DImageIOPluginHandler FileHandler; 
}; 


typedef TSeededWS<3> C3DSeededWS; 
typedef TSeededWSFilterPlugin<3> C3DSeededWSFilterPlugin;


NS_MIA_END
