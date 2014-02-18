/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <sstream>

NS_MIA_BEGIN


template <typename T> 
TFullCostList<T>::TFullCostList():
	TFullCost<T>(1.0)
{
}

template <typename T> 
void TFullCostList<T>::push(typename TFullCost<T>::Pointer cost)
{
	m_costs.push_back(cost); 
}

template <typename T> 
bool TFullCostList<T>::do_has(const char *property) const
{
	TRACE_FUNCTION; 
	bool result = !m_costs.empty(); 
	if (!result) 
		cvwarn() << "No cost functions given\n"; 
	auto ic = m_costs.begin(); 
	while (result && ic != m_costs.end()) {
		result &= (*ic)->has(property); 
		if (!result) {
			cvwarn() << "Cost '"<< (*ic)->get_init_string() 
				 << "' doesn't provide property '" 
				 << property << "'\n"; 
		}
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
	for (auto i = m_costs.begin(); i != m_costs.end(); ++i) {
		std::fill(tmp.begin(), tmp.end(), 0.0); 
		double h = (*i)->evaluate(t, tmp); 
		msg << h << "("<< (*i)->get_init_string() << ") "; 
		result += h; 
		std::transform(gradient.begin(), gradient.end(), tmp.begin(), gradient.begin(), 
			       [](double x, double y){return x+y;}); 
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
	for (auto i = m_costs.begin(); i != m_costs.end(); ++i) {
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
	for (auto i = m_costs.begin(); i != m_costs.end(); ++i) {
		result += (*i)->cost_value(); 
	}
	return result; 
}

template <typename T> 
void TFullCostList<T>::do_set_size()
{
	for (auto i = m_costs.begin(); i != m_costs.end(); ++i) 
		(*i)->set_size(this->get_current_size()); 
}

template <typename T> 
bool TFullCostList<T>::do_get_full_size(Size& size) const
{
	bool result = true; 
	for (auto i = m_costs.begin(); i != m_costs.end() && result; ++i) 
		result = (*i)->get_full_size(size); 
	return result; 
}

template <typename T> 
void TFullCostList<T>::do_reinit()
{
	for (auto i = m_costs.begin(); i != m_costs.end(); ++i) 
		(*i)->reinit(); 
}

NS_MIA_END
