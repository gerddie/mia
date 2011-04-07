/* -*- mia-c++  -*-
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

/*
  CTParameter
  template implementation file - include after parameter.hh, and only if you define a new parameter type

*/

#include <sstream>
#include <stdexcept>

#include <mia/core/msgstream.hh>
NS_MIA_BEGIN

/**
   \class CTParameter  
   A parameter of a certain value type
*/


template <typename T, const char * const TS> 
CTParameter<T, TS>::CTParameter(T& value,  bool required, const char *descr):
	CParameter(TS, required, descr),
	_M_value(value)
{
	
}

template <typename T, const char * const TS> 
void CTParameter<T, TS>::do_descr(std::ostream& os) const
{
	os << " (default=" <<  _M_value << ") ";
}


template <typename T, const char * const TS> 
bool CTParameter<T, TS>::do_set(const std::string& str_value)
{
	char c; 
	std::istringstream s(str_value); 
	s >> _M_value; 
	if (s.fail()) 
		throw std::invalid_argument(errmsg(str_value)); 
	while (!s.eof() && s.peek() == ' ') 
		s >> c; 
	if (!s.eof()) 
		throw std::invalid_argument(errmsg(str_value)); 
	
	adjust(_M_value); 
	return true; 
}

template <typename T, const char * const TS> 
void CTParameter<T,TS>::adjust(T& /*value*/)
{
}


template <typename T, const char * const TS> 
TRangeParameter<T,TS>::TRangeParameter(T& value, T min, T max, bool required, const char *descr):
	CTParameter<T, TS>(value, required, descr),
	_M_min(min), 
	_M_max(max)
{
	if (_M_min > _M_max) 
		throw std::invalid_argument("TRangeParameter: min > max not allowed"); 
}

template <typename T, const char * const TS> 
void TRangeParameter<T,TS>::adjust(T& value)
{
	if (value < _M_min) {
		cvwarn() << "TRangeParameter<T,TS>: adjust " << value <<" to lower bound " << _M_min << "\n"; 
		value = _M_min; 
	}

	
	if (value > _M_max) {
		cvwarn() << "TRangeParameter<T,TS>: adjust " << value <<" to upper bound " << _M_max << "\n"; 
		value = _M_max; 
	}
}

template <typename T, const char * const TS> 
void TRangeParameter<T,TS>::do_descr(std::ostream& os) const
{
	CTParameter<T, TS>::do_descr(os); 
	os << " in [" << _M_min << "," << _M_max << "] ";
}

NS_MIA_END
