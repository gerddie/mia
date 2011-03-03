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

#include <limits>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN
template <typename T> 
TDivCurlFullCost<T>::TDivCurlFullCost(double weight_div, double weight_curl, double weight):
	TFullCost<T>(weight), 
	_M_weight_div(weight_div), 
	_M_weight_curl(weight_curl), 
	_M_size_scale(1.0)
{
	this->add(::mia::property_gradient); 
}

template <typename T> 
double TDivCurlFullCost<T>::do_evaluate(const T& t, CDoubleVector& gradient) const
{
	assert(t.get_size() == this->get_current_size()); 
	double result = t.get_divcurl_cost(_M_size_scale * _M_weight_div, _M_size_scale *_M_weight_curl, gradient); 
	cvdebug() << "TDivCurlFullCost<T>::value = " << result << "\n"; 
	transform(gradient.begin(), gradient.end(), gradient.begin(), -1.0 * boost::lambda::_1); 
	return result; 
}

template <typename T> 
double TDivCurlFullCost<T>::do_value(const T& t) const
{
	double result = t.get_divcurl_cost(_M_size_scale * _M_weight_div, _M_size_scale * _M_weight_curl); 
	cvdebug() << "TDivCurlFullCost<T>::value = " << result << "\n"; 
	return result; 
}

template <typename T> 
double TDivCurlFullCost<T>::do_value() const
{
	cvwarn() << "Requesting DivCurl cost without a transformation doesn't make sense\n"; 
	return 0.0; 
}

template <typename T> 
void TDivCurlFullCost<T>::do_set_size()
{
	_M_size_scale = 1.0 / (this->get_current_size().product()); 
}

template <typename T> 
TDivcurlFullCostPlugin<T>::TDivcurlFullCostPlugin():
	TFullCostPlugin<T>("divcurl"), 
	_M_div(1.0), 
	_M_curl(1.0)
{
	this->add_parameter("div", new CFloatParameter(_M_div, 0.0f, numeric_limits<float>::max(), 
						 false, "penalty weight on divergence"));
	this->add_parameter("curl", new CFloatParameter(_M_curl, 0.0f, numeric_limits<float>::max(), 
						  false, "penalty weight on curl"));
}

template <typename T> 
typename TFullCostPlugin<T>::ProductPtr TDivcurlFullCostPlugin<T>::do_create(float weight) const
{
	cvdebug() << "create C2DDivCurlFullCost with weight= " << weight 
		  << " div=" << _M_div << " curl=" << _M_curl << "\n"; 
		
	return typename TFullCostPlugin<T>::ProductPtr(new TDivCurlFullCost<T>(this->_M_div, this->_M_curl, weight)); 
}

template <typename T> 
const std::string TDivcurlFullCostPlugin<T>::do_get_descr() const
{
	return "divcurl penalty cost function"; 
}

NS_MIA_END



