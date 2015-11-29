/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_3d_maskedcost_ssd_hh
#define mia_3d_maskedcost_ssd_hh

#include <mia/3d/maskedcost.hh>

#define NS mia_3dcost_ssd_masked
#include <mia/template/ssd_masked.hh>

NS_BEGIN(NS)

typedef TSSDMaskedImageCost<mia::C3DMaskedImageCost> C3DSSDMaskedImageCost;
typedef TSSDMaskedImageCostPlugin<mia::C3DMaskedImageCostPlugin, mia::C3DMaskedImageCost> C3DSSDMaskedCostPlugin; 

NS_END


#endif
