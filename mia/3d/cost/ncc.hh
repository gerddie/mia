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

#ifndef mia_3d_cost_ncc_hh
#define mia_3d_cost_ncc_hh

#include <mia/3d/cost.hh>
#include <mia/3d/filter.hh>

#define NS mia_3d_ncc

NS_BEGIN(NS)

class CNCC3DImageCost: public mia::C3DImageCost
{
public:
       typedef mia::C3DImageCost::Data Data;
       typedef mia::C3DImageCost::Force Force;

       CNCC3DImageCost();
private:
       virtual double do_value(const Data& a, const Data& b) const;
       virtual double do_evaluate_force(const Data& a, const Data& b, Force& force) const;
       mia::P3DFilter m_copy_to_double;
};

class CNCC3DImageCostPlugin: public mia::C3DImageCostPlugin
{
public:
       CNCC3DImageCostPlugin();
       mia::C3DImageCost *do_create() const;
private:
       const std::string do_get_descr() const;
};

NS_END

#endif
