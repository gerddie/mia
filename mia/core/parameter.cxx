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

/*
  CTParameter
  template implementation file - include after parameter.hh, and only if you define a new parameter type

*/

#include <sstream>
#include <stdexcept>
#include <mia/core/tools.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/typedescr.hh>

NS_MIA_BEGIN

/**
   \class CTParameter  
   A parameter of a certain value type
*/


template <typename T> 
CTParameter<T>::CTParameter(T& value,  bool required, const char *descr):
	CParameter(__type_descr<T>::value, required, descr),
	m_value(value), 
	m_default_value(value)
{
	
}

template <typename T> 
void CTParameter<T>::do_descr(std::ostream& /*os*/) const
{
}

template <typename T>
struct __dispatch_parameter_do_set {
	static bool apply (const std::string& str_value, T& value) {
		char c; 
		std::istringstream s(str_value); 
		s >> value; 
		if (s.fail()) 
			throw create_exception<std::invalid_argument>("Value '", str_value, "' not allowed"); 
		while (!s.eof() && s.peek() == ' ') 
			s >> c; 
		if (!s.eof()) 
			throw create_exception<std::invalid_argument>("Value '", str_value, "' not allowed"); 
		return true; 
	}
};  

template <>
struct __dispatch_parameter_do_set<std::string> {
	static bool apply (const std::string& str_value, std::string& value) {
		value = str_value;
		return true; 
	}
}; 


template <typename T> 
bool CTParameter<T>::do_set(const std::string& str_value)
{
	bool retval = __dispatch_parameter_do_set<T>::apply(str_value, m_value); 
	adjust(m_value); 
	return retval; 
}

template <typename T>
void CTParameter<T>::do_reset()
{
	m_value = m_default_value;
}

template <typename T> 
void CTParameter<T>::adjust(T& /*value*/)
{
}

template <typename T> 
std::string CTParameter<T>::do_get_value_as_string() const
{
	std::ostringstream s; 
	s << m_value; 
	return s.str(); 
}

template <typename T> 
std::string CTParameter<T>::do_get_default_value() const
{
	std::ostringstream s; 
	s << m_default_value; 
	return s.str(); 
}

template <typename T> 
TRangeParameter<T>::TRangeParameter(T& value, T min, T max, bool required, const char *descr):
	CTParameter<T>(value, required, descr),
	m_min(min), 
	m_max(max)
{
	if (m_min > m_max) 
		throw create_exception<std::invalid_argument>("Parameter '",descr,"' TRangeParameter<T,", __type_descr<T>::value , ">: min(" 
						    , m_min ,") > max (", m_max , ")  not allowed"); 
}

template <typename T> 
void TRangeParameter<T>::adjust(T& value)
{
	if (value < m_min) {
		cvwarn() << "TRangeParameter<T>: adjust " << value <<" to lower bound " << m_min << "\n"; 
		value = m_min; 
	}

	
	if (value > m_max) {
		cvwarn() << "TRangeParameter<T>: adjust " << value <<" to upper bound " << m_max << "\n"; 
		value = m_max; 
	}
}

template <typename T> 
void TRangeParameter<T>::do_descr(std::ostream& os) const
{
	CTParameter<T>::do_descr(os); 
	os << " in [" << m_min << "," << m_max << "] ";
}

template <typename T> 
void TRangeParameter<T>::do_get_help_xml(xmlpp::Element& self) const
{
	auto dict = self.add_child("range"); 
	dict->set_attribute("min", to_string<T>(m_min)); 
	dict->set_attribute("max", to_string<T>(m_max)); 
}


NS_MIA_END
