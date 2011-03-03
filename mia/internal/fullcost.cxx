/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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


NS_MIA_BEGIN

template <typename T> 
const char *TFullCost<T>::type_descr = T::dim_descr; 

template <typename T> 
const char *TFullCost<T>::value = "fullcost";

template <typename T> 
TFullCost<T>::TFullCost(double weight):
	_M_weight(weight)
{
}

template <typename T> 
double TFullCost<T>::evaluate(const T& t, CDoubleVector& gradient) const
{
	assert(_M_current_size == t.get_size()); 
	
	double result = _M_weight * do_evaluate(t, gradient); 
	transform(gradient.begin(), gradient.end(), gradient.begin(), _M_weight * boost::lambda::_1); 
	return result; 
}

template <typename T> 
double TFullCost<T>::cost_value(const T& t) const 
{
	return _M_weight * do_value(t); 
}

template <typename T> 
double TFullCost<T>::cost_value() const 
{
	return _M_weight * do_value(); 
}
	

template <typename T> 
double TFullCost<T>::get_weight() const
{
	return _M_weight; 
}

template <typename T> 
const typename TFullCost<T>::Size& TFullCost<T>::get_current_size() const
{
	return _M_current_size; 
}

template <typename T> 
void TFullCost<T>::reinit()
{
	do_reinit(); 
}

template <typename T> 
void TFullCost<T>::set_size(const Size& size)
{
	if (_M_current_size != size) {
		_M_current_size = size; 
		do_set_size(); 
	}
}

template <typename T> 
void TFullCost<T>::do_reinit()
{
}

template <typename T> 
TFullCostPlugin<T>::TFullCostPlugin(const char *name):
	TFactory<TFullCost<T> >(name), 
	_M_weight(1.0)
{
	this->add_parameter("weight", new CFloatParameter(_M_weight, -1e+10f, 1e+10f,
						    false, "weight of cost function"));
}
	
template <typename T> 
typename TFullCostPlugin<T>::ProductPtr TFullCostPlugin<T>::do_create() const
{
	return do_create(_M_weight); 
}

NS_MIA_END
