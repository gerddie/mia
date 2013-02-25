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

#include <mia/3d/filter.hh>
#include <mia/3d/imageio.hh>
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
