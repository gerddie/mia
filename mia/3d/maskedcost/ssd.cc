/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/3d/maskedcost/ssd.hh>
#include <mia/template/ssd_masked.cxx>

NS_BEGIN(NS)

NS_MIA_USE;
using namespace std;
using namespace boost;

template class TSSDMaskedImageCost<mia::C3DMaskedImageCost>;
template class TSSDMaskedImageCostPlugin<mia::C3DMaskedImageCostPlugin, mia::C3DMaskedImageCost>;

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C3DSSDMaskedCostPlugin();
}

NS_END
