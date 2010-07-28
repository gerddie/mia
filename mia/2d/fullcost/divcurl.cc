/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <mia/2d/fullcost/divcurl.hh>
NS_MIA_BEGIN

C2DDivCurlFullCost::C2DDivCurlFullCost(double weight_div, double weight_curl, double weight):
	C2DFullCost(weight), 
	_M_weight_div(weight_div), 
	_M_weight_curl(weight_curl)
{

}

double C2DDivCurlFullCost::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	assert(t.get_size() == get_current_size()); 
	return t.get_divcurl_cost(_M_weight_div, _M_weight_curl, gradient); 
}

void C2DDivCurlFullCost::do_set_size()
{
}



/*
	add_parameter("div", new CFloatParameter(_M_div, 0.0, numeric_limits<float>::max(), 
						 false, "penalty weight on divergence"));
	add_parameter("curl", new CFloatParameter(_M_curl, false, "penalty weight on curl"));
*/

NS_MIA_END
