/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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

#include <mia/3d/fullcost/divcurl.hh>
#include <mia/internal/divcurl.cxx>

NS_MIA_BEGIN

template class TDivcurlFullCostPlugin<C3DTransformation>; 
template class  TDivCurlFullCost<C3DTransformation>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDivCurlFullCostPlugin();
}

NS_MIA_END



