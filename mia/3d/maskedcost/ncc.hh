/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_3d_maskedcost_ncc_hh
#define mia_3d_maskedcost_ncc_hh

#include <mia/3d/maskedcost.hh>

#define NS mia_3d_maskedncc

NS_BEGIN(NS)

class CNCC3DImageCost: public mia::C3DMaskedImageCost {
public: 	
	typedef mia::C3DMaskedImageCost::Data Data; 
	typedef mia::C3DMaskedImageCost::Force Force; 
	typedef mia::C3DMaskedImageCost::Mask Mask; 

	CNCC3DImageCost();
private: 
	virtual double do_value(const Data& a, const Data& b, const Mask& m) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const; 
};

class CNCC3DImageCostPlugin: public mia::C3DMaskedImageCostPlugin {
public: 
	CNCC3DImageCostPlugin();
	mia::C3DMaskedImageCost *do_create() const;
private: 
	const std::string do_get_descr() const; 
};

NS_END

#endif 
