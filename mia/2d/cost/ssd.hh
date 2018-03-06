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

#ifndef mia_2d_cost_ssd_hh
#define mia_2d_cost_ssd_hh

#include <mia/2d/cost.hh>

// the actual implementation is here, shared between
// 2d and 3d

#define NS mia_2dcost_ssd

#include <mia/template/ssd.hh>

NS_BEGIN(mia_2dcost_ssd)


typedef TSSDCost<mia::C2DImageCost> C2DSSDCost;

class C2DSSDCostPlugin: public TSSDCostPlugin<mia::C2DImageCostPlugin, C2DSSDCost>
{
private:
       virtual const std::string do_get_descr()const;
};

NS_END

#endif

