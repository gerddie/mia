/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef mia_2d_cost_divcurl_hh
#define mia_2d_cost_divcurl_hh

#include <mia/2d/costbase.hh>


NS_BEGIN(mia_2dcost_divcurl)


struct C2DDivCurlCostImpl;

class C2DDivCurlCost: public mia::C2DCostBase {
public:
	C2DDivCurlCost(float weight, float divergence, float curl);
	~C2DDivCurlCost();
private:
	virtual double do_evaluate(const mia::C2DTransformation& t, mia::C2DFVectorfield& force) const;
	struct C2DDivCurlCostImpl *impl;
};

NS_END

#endif
