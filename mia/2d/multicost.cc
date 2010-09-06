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

#include <boost/lambda/lambda.hpp>
#include <mia/2d/multicost.hh>

NS_MIA_BEGIN
using boost::lambda::_1; 
using boost::lambda::_2; 


C2DFullCostList::C2DFullCostList():
	C2DFullCost(1.0)
{
}

void C2DFullCostList::push(P2DFullCost cost)
{
	_M_costs.push_back(cost); 
}

bool C2DFullCostList::do_has(const char *property) const
{
	bool result = !_M_costs.empty(); 
	auto ic = _M_costs.begin(); 
	while (result && ic != _M_costs.end()) {
		result &= (*ic)->has(property); 
		++ic; 
	}
	return result; 
}

double C2DFullCostList::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	double  result = 0; 
	gsl::DoubleVector tmp(gradient.size()); 
	stringstream msg; 
	msg << "Cost: "; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		double h = (*i)->evaluate(t, tmp); 
		msg << h << "("<< (*i)->get_init_string() << ") "; 
		result += h; 
		transform(gradient.begin(), gradient.end(), tmp.begin(), gradient.begin(), _1 + _2); 
	}
	cvdebug() << msg.str() << " = " << result << "\n"; 
	return result; 
}

double C2DFullCostList::do_value(const C2DTransformation& t) const
{
	double  result = 0; 
	stringstream msg; 
	msg << "Cost: "; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		double h = (*i)->cost_value(t); 
		msg << h << "("<< (*i)->get_init_string() << ") "; 
		result += h; 
	}
	cvdebug() << msg.str() << " = " << result << "\n"; 
	return result; 
}

double C2DFullCostList::do_value() const
{
	double  result = 0; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		result += (*i)->cost_value(); 
	}
	return result; 
}

void C2DFullCostList::do_set_size()
{
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) 
		(*i)->set_size(get_current_size()); 
}

void C2DFullCostList::do_reinit()
{
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) 
		(*i)->reinit(); 
}

NS_MIA_END
