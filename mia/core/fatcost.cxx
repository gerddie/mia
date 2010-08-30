/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <mia/core/fatcost.hh>

NS_MIA_BEGIN
template <typename T, typename F>
TFatCost<T,F>::TFatCost(std::shared_ptr<Data > src, std::shared_ptr<Data > ref, 
			std::shared_ptr<Interpolator >  ipf, float weight):
	_M_src(src), 
	_M_ref(ref), 
	_M_ipf(ipf), 
	_M_floating(src), 
	_M_weight(weight),
	_M_cost_valid(false)
{
}


template <typename T, typename F>
TFatCost<T,F>::~TFatCost()
{
}

template <typename T, typename F>	
double TFatCost<T,F>::value() const 
{
	TRACE("TFatCost<T,F>::value"); 
	if (!_M_cost_valid) {
		_M_cost = do_value() * _M_weight; 
		_M_cost_valid = true; 
	}
	return _M_cost; 
}

template <typename T, typename F>
double TFatCost<T,F>::evaluate_force(F& force) const
{
	TRACE("TFatCost<T,F>::evaluate_force"); 
	_M_cost = do_evaluate_force(force); 
	_M_cost_valid = true; 
	return _M_cost; 
}

template <typename T, typename F>		
void TFatCost<T,F>::transform(const T& t)
{
	TRACE("TFatCost<T,F>::transform"); 
	_M_cost_valid = false; 
	_M_floating = t(*_M_src, *_M_ipf); 
}


NS_MIA_END
