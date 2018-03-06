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

#ifndef mia_2d_cost_mi_hh
#define mia_2d_cost_mi_hh

#include <mia/2d/cost.hh>

#define NS mia_2dcost_mi
#include <mia/template/mi.hh>

NS_BEGIN(NS)

typedef TMIImageCost<mia::C2DImageCost> C2DMIImageCost;
typedef TMIImageCostPlugin<mia::C2DImageCostPlugin, mia::C2DImageCost> C2DMICostPlugin;

NS_END


#endif
