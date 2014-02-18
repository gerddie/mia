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

#include <type_traits> 
#include <queue> 
#include <mia/2d/filter/seededwatershed.hh>

NS_MIA_BEGIN 

template class  TSeededWS<2>; 
template class  TSeededWSFilterPlugin<2>;


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSeededWSFilterPlugin();
}


NS_END
