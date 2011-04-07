/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
	m_value(value)
{
	
}

template <typename T, const char * const TS> 
void CTParameter<T, TS>::do_descr(std::ostream& os) const
{
	os << " (default=" <<  m_value << ") ";
}


template <typename T, const char * const TS> 
bool CTParameter<T, TS>::do_set(const std::string& str_value)
{
	char c; 
	std::istringstream s(str_value); 
	s >> m_value; 
	if (s.fail()) 
		throw std::invalid_argument(errmsg(str_value)); 
	while (!s.eof() && s.peek() == ' ') 
		s >> c; 
	if (!s.eof()) 
		throw std::invalid_argument(errmsg(str_value)); 
	
	adjust(m_value); 
	return true; 
}

template <typename T, const char * const TS> 
void CTParameter<T,TS>::adjust(T& /*value*/)
{
}


template <typename T, const char * const TS> 
TRangeParameter<T,TS>::TRangeParameter(T& value, T min, T max, bool required, const char *descr):
	CTParameter<T, TS>(value, required, descr),
	m_min(min), 
	m_max(max)
{
	if (m_min > m_max) 
		throw std::invalid_argument("TRangeParameter: min > max not allowed"); 
}

template <typename T, const char * const TS> 
void TRangeParameter<T,TS>::adjust(T& value)
{
	if (value < m_min) {
		cvwarn() << "TRangeParameter<T,TS>: adjust " << value <<" to lower bound " << m_min << "\n"; 
		value = m_min; 
	}

	
	if (value > m_max) {
		cvwarn() << "TRangeParameter<T,TS>: adjust " << value <<" to upper bound " << m_max << "\n"; 
		value = m_max; 
	}
}

template <typename T, const char * const TS> 
void TRangeParameter<T,TS>::do_descr(std::ostream& os) const
{
	CTParameter<T, TS>::do_descr(os); 
	os << " in [" << m_min << "," << m_max << "] ";
}

NS_MIA_END
