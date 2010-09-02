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
#include <mia/2d/fullcost/divcurl.hh>
NS_MIA_BEGIN
using namespace std; 

C2DDivCurlFullCost::C2DDivCurlFullCost(double weight_div, double weight_curl, double weight):
	C2DFullCost(weight), 
	_M_weight_div(weight_div), 
	_M_weight_curl(weight_curl), 
	_M_size_scale(1.0)
{
	this->add(::mia::property_gradient); 
}

double C2DDivCurlFullCost::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	assert(t.get_size() == get_current_size()); 
	double result = t.get_divcurl_cost(_M_size_scale * _M_weight_div, _M_size_scale *_M_weight_curl, gradient); 
	cvinfo() << "DivCurl=" << result << "\n"; 
	return result; 
}

double C2DDivCurlFullCost::do_value(const C2DTransformation& t) const
{
	double result = t.get_divcurl_cost(_M_size_scale * _M_weight_div, _M_size_scale * _M_weight_curl); 
	cvdebug() << "C2DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C2DDivCurlFullCost::do_value() const
{
	return 0.0; 
}

void C2DDivCurlFullCost::do_set_size()
{
	_M_size_scale = 1.0 / (get_current_size().x * get_current_size().y); 
}


class C2DDivcurlFullCostPlugin: public C2DFullCostPlugin {
public: 
	C2DDivcurlFullCostPlugin(); 
private: 
	C2DFullCostPlugin::ProductPtr do_create(float weight) const;
	const std::string do_get_descr() const;
	float _M_div;
	float _M_curl;
}; 

C2DDivcurlFullCostPlugin::C2DDivcurlFullCostPlugin():
	C2DFullCostPlugin("divcurl"), 
	_M_div(1.0), 
	_M_curl(1.0)
{
	add_parameter("div", new CFloatParameter(_M_div, 0.0f, numeric_limits<float>::max(), 
						 false, "penalty weight on divergence"));
	add_parameter("curl", new CFloatParameter(_M_curl, 0.0f, numeric_limits<float>::max(), 
						  false, "penalty weight on curl"));
}

C2DFullCostPlugin::ProductPtr C2DDivcurlFullCostPlugin::do_create(float weight) const
{
	return C2DFullCostPlugin::ProductPtr(new C2DDivCurlFullCost(_M_div,  _M_curl, weight)); 
}

const std::string C2DDivcurlFullCostPlugin::do_get_descr() const
{
	return "divcurl penalty cost function"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDivcurlFullCostPlugin();
}

NS_MIA_END



