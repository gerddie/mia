/* -*- mia-c++  -*-
 *
 * Copyright (c) 2010 Gert Wollny 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <mia/2d/cost/divcurl.hh>

NS_BEGIN(mia_2dcost_divcurl)

using namespace mia; 

struct C2DDivCurlCostImpl {

	C2DDivCurlCostImpl(float _divergence, float _curl); 
	
	float divergence; 
	float curl; 
	
}; 

C2DDivCurlCost::C2DDivCurlCost(float weight, float divergence, float curl):
	C2DCostBase(weight)
{
	impl = new C2DDivCurlCostImpl(divergence, curl); 
}

C2DDivCurlCost::~C2DDivCurlCost()
{
	delete impl; 
}

double C2DDivCurlCost::do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{
	return 0.0; 
}

C2DDivCurlCostImpl::C2DDivCurlCostImpl(float _divergence, float _curl):
	divergence(_divergence), 
	curl(_curl)
{
}

NS_END
