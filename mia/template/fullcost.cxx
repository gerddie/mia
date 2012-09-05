/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


NS_MIA_BEGIN

template <typename T> 
const char *TFullCost<T>::data_descr = T::dim_descr; 

template <typename T> 
const char *TFullCost<T>::type_descr = "fullcost";

template <typename T> 
TFullCost<T>::TFullCost(double weight):
	m_weight(weight)
{
	add(property_gradient); 
}

template <typename T> 
double TFullCost<T>::evaluate(const T& t, CDoubleVector& gradient) const
{
	assert(m_current_size == t.get_size()); 
	
	double result = m_weight * do_evaluate(t, gradient); 
	std::transform(gradient.begin(), gradient.end(), gradient.begin(), 
		  [this](typename CDoubleVector::value_type& v) { return m_weight * v;}); 
	return result; 
}

template <typename T> 
double TFullCost<T>::cost_value(const T& t) const 
{
	return m_weight * do_value(t); 
}

template <typename T> 
double TFullCost<T>::cost_value() const 
{
	return m_weight * do_value(); 
}
	

template <typename T> 
double TFullCost<T>::get_weight() const
{
	return m_weight; 
}

template <typename T> 
const typename TFullCost<T>::Size& TFullCost<T>::get_current_size() const
{
	return m_current_size; 
}

template <typename T> 
void TFullCost<T>::reinit()
{
	do_reinit(); 
}

template <typename T> 
void TFullCost<T>::set_size(const Size& size)
{
	m_current_size = size; 
	do_set_size(); 
}

template <typename T> 
bool TFullCost<T>::get_full_size(Size& size) const
{
	return do_get_full_size(size); 
}

template <typename T> 
bool TFullCost<T>::do_get_full_size(Size& size) const
{
	return (m_current_size == size);  
}

template <typename T> 
void TFullCost<T>::do_reinit()
{
}

template <typename T> 
TFullCostPlugin<T>::TFullCostPlugin(const char *name):
	TFactory<TFullCost<T> >(name), 
	m_weight(1.0)
{
	this->add_parameter("weight", new CFloatParameter(m_weight, -1e+10f, 1e+10f,
						    false, "weight of cost function"));
}
	
template <typename T> 
TFullCost<T> *TFullCostPlugin<T>::do_create() const
{
	return do_create(m_weight); 
}

NS_MIA_END
