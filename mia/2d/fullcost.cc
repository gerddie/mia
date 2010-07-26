/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#include <boost/lambda/lambda.hpp>
#include <mia/core/export_handler.hh>
#include <mia/2d/fullcost.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN
using boost::lambda::_1; 

const char *C2DFullCost::type_descr = "2d";
const char *C2DFullCost::value = "fullcost";

C2DFullCost::C2DFullCost(double weight):
	_M_weight(weight), 
	_M_current_size(0,0)
{
}

double C2DFullCost::evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	assert(_M_current_size == t.get_size()); 
	
	double result = _M_weight * do_evaluate(t, gradient); 
	transform(gradient.begin(), gradient.end(), gradient.begin(), _M_weight * _1); 
	return result; 
}

double C2DFullCost::get_weight() const
{
	return _M_weight; 
}

const C2DBounds& C2DFullCost::get_current_size() const
{
	return _M_current_size; 
}

void C2DFullCost::set_size(const C2DBounds& size)
{
	if (_M_current_size != size) {
		_M_current_size = size; 
		do_set_size(); 
	}
}

template class EXPORT_HANDLER TPlugin<C2DFullCost, C2DFullCost>;
template class EXPORT_HANDLER TFactory<C2DFullCost, C2DFullCost, C2DFullCost>;
template class EXPORT_HANDLER TFactoryPluginHandler<C2DFullCostPlugin>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C2DFullCostPlugin> >;
template class TPluginHandler<C2DFullCostPlugin>;

NS_MIA_END
