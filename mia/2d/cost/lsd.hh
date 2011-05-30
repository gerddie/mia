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

#ifndef mia_2d_cost_ls_hh
#define mia_2d_cost_ls_hh

#include <mia/2d/cost.hh>

#define NS mia_2dcost_lsd
#include <mia/internal/lsd.hh>

NS_BEGIN(mia_2dcost_lsd)

typedef TLSDImageCost<mia::C2DImageCost> C2DLSDImageCost;
typedef TLSDImageCostPlugin<mia::C2DImageCostPlugin, C2DLSDImageCost> C2DLSDCostPlugin; 

NS_END


#endif
