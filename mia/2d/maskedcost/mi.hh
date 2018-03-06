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

#include <mia/2d/maskedcost.hh>

#define NS mia_2dcost_mi_masked
#include <mia/template/mi_masked.hh>

NS_BEGIN(NS)

typedef TMIMaskedImageCost<mia::C2DMaskedImageCost> C2DMIMaskedImageCost;
typedef TMIMaskedImageCostPlugin<mia::C2DMaskedImageCostPlugin, mia::C2DMaskedImageCost> C2DMIMaskedCostPlugin;

NS_END


#endif
