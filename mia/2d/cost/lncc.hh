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

#ifndef mia_2d_maskedcost_lncc_hh
#define mia_2d_maskedcost_lncc_hh

#include <mia/2d/cost.hh>

#define NS mia_2d_lncc

NS_BEGIN(NS)

class CLNCC2DImageCost: public mia::C2DImageCost {
public: 	
	typedef mia::C2DImageCost::Data Data; 

	CLNCC2DImageCost(int hw);
private: 
	virtual double do_value(const Data& a, const Data& b) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, Force& force) const; 
        int m_hwidth; 
};

class CLNCC2DImageCostPlugin: public mia::C2DImageCostPlugin {
public: 
	CLNCC2DImageCostPlugin();
	mia::C2DImageCost *do_create() const;
private: 
	const std::string do_get_descr() const; 
        unsigned int m_hw; 
};

NS_END

#endif 
