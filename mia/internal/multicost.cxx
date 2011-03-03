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

#include <sstream>
#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN


template <typename T> 
TFullCostList<T>::TFullCostList():
	TFullCost<T>(1.0)
{
}

template <typename T> 
void TFullCostList<T>::push(typename TFullCost<T>::Pointer cost)
{
	_M_costs.push_back(cost); 
}

template <typename T> 
bool TFullCostList<T>::do_has(const char *property) const
{
	bool result = !_M_costs.empty(); 
	auto ic = _M_costs.begin(); 
	while (result && ic != _M_costs.end()) {
		result &= (*ic)->has(property); 
		++ic; 
	}
	return result; 
}

template <typename T> 
double TFullCostList<T>::do_evaluate(const T& t, CDoubleVector& gradient) const
{
	double  result = 0; 
	CDoubleVector tmp(gradient.size()); 
	std::stringstream msg; 
	msg << "Cost: "; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		fill(tmp.begin(), tmp.end(), 0.0); 
		double h = (*i)->evaluate(t, tmp); 
		msg << h << "("<< (*i)->get_init_string() << ") "; 
		result += h; 
		transform(gradient.begin(), gradient.end(), tmp.begin(), gradient.begin(), 
			  boost::lambda::_1 + boost::lambda::_2); 
	}
	cvinfo() << msg.str() << " = " << result << "\n"; 
	return result; 
}

template <typename T> 
double TFullCostList<T>::do_value(const T& t) const
{
	double  result = 0; 
	std::stringstream msg; 
	msg << "Cost: "; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		double h = (*i)->cost_value(t); 
		msg << h << "("<< (*i)->get_init_string() << ") "; 
		result += h; 
	}
	cvinfo() << msg.str() << " = " << result << "\n"; 
	return result; 
}

template <typename T> 
double TFullCostList<T>::do_value() const
{
	double  result = 0; 
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) {
		result += (*i)->cost_value(); 
	}
	return result; 
}

template <typename T> 
void TFullCostList<T>::do_set_size()
{
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) 
		(*i)->set_size(this->get_current_size()); 
}

template <typename T> 
void TFullCostList<T>::do_reinit()
{
	for (auto i = _M_costs.begin(); i != _M_costs.end(); ++i) 
		(*i)->reinit(); 
}

NS_MIA_END
