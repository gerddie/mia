/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
		assert(!str_value.empty()); 
		if (std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed) {
			std::string::size_type startpos = str_value.find_first_not_of(" \t");
			if (startpos  == std::string::npos) {
				throw create_exception<std::invalid_argument>("Trying to set a parameter from an string "
									      "that is comprised of whitespaces only"); 
			}
			if (str_value[startpos] == '-') {
				throw create_exception<std::invalid_argument>("Try setting an unsigned value with negative value ", str_value); 
			}
		}
		

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

template <typename T>
struct __dispatch_parameter_do_set<std::vector<T> >  {
	static bool apply (const std::string& str_value, std::vector<T>& value) {
		assert(!str_value.empty()); 
		std::string h(str_value);
		unsigned int n = 1; 
		
		// count input values 
		for(auto hb = h.begin(); hb != h.end(); ++hb)
			if (*hb == ',') {
				*hb = ' ';
				++n; 
			}
		
		if (!value.empty()) {
			if (n > value.size()) {
				throw create_exception<std::invalid_argument>("Expect only ", value.size(),  
									      " coma separated values, but '", 
									      str_value, "' provides ", n);
			}
		}else{
			value.resize(n); 
		}

				
                std::istringstream sval(h);
		auto i =  value.begin(); 
		while (sval.good()) {
			sval >> *i;
			++i; 
		}
                return sval.eof();
	}
};  

template <>
struct __dispatch_parameter_do_set<std::string> {
	static bool apply (const std::string& str_value, std::string& value) {
		assert(!str_value.empty()); 
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
	auto str = s.str(); 
	if (str.find(',') != std::string::npos) {
		std::ostringstream s2; 
		s2 << '[' << str << ']'; 
		str =  s2.str(); 
	}
	return str; 
}

template <typename T> 
std::string CTParameter<T>::do_get_default_value() const
{
	std::ostringstream s; 
	s << m_default_value; 
	auto str = s.str(); 
	if (str.find(',') != std::string::npos) {
		std::ostringstream s2; 
		s2 << '[' << str << ']'; 
		str =  s2.str(); 
	}
	return str; 
}

NS_MIA_END
