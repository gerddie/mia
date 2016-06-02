/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_core_svector_hh
#define mia_core_svector_hh

#include <istream>
#include <ostream>
#include <sstream>
#include <vector>
#include <stdexcept>

#include <mia/core/errormacro.hh>
#include <mia/core/typedescr.hh>

NS_MIA_BEGIN

/**
   \ingroup logging
   \brief implements the direct streaming of std::vectors. 
*/
template <typename T> 
std::ostream& operator << (std::ostream& os, const std::vector<T>& v) {
	auto i = v.begin();
	auto e = v.end(); 

	if (i != e)
		os << *i++;
	while (i != e)
		os << "," << *i++;
	return os; 
}

template <typename T> 
struct __dispatch_translate {
	static bool apply(const std::string& str, T& v){
		char c; 
		std::istringstream s(str); 
		s >> v;
		if (s.fail())
			return false; 
		while (!s.eof() && s.peek() == ' ') 
			s >> c; 
		return s.eof(); 
	}
}; 

template <> 
struct __dispatch_translate<std::string> {
	static bool apply(const std::string& s, std::string& str){
		str = s; 
		return true; 
	}
}; 


template <typename T> 
std::istream&  operator >> (std::istream& is, std::vector<T>& v)
{
	std::vector<T> values; 
	std::string token; 
	T val; 
	
	while(std::getline(is, token, ',')) {
		if (__dispatch_translate<T>::apply(token, val))
			values.push_back(val); 
		else {
			throw create_exception<std::invalid_argument>("Reading vector: value, '", token, 
								      "' could not be translate to ", 
								      mia::__type_descr<T>::value);
		}
	}
	
	if (!v.empty() && v.size() != values.size()) {
			throw create_exception<std::invalid_argument>("Reading vector: expected ", 
								 v.size(), " values, but got ", values.size()); 
	}
	v.swap(values); 
	return is; 
}


NS_MIA_END

#endif 

