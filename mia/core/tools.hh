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

#ifndef mia_core_tools_hh
#define mia_core_tools_hh

#include <sstream>
#include <string>
#include <boost/call_traits.hpp>
#include <memory>
#include <cmath>
#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   \brief A helper class to make it possible to store a non-pointer object or a pointer that must not be freed 
   in a shared pointer 
   \tparam the type type be stored 
*/

template <typename T> 
struct void_destructor {
	virtual void operator () (T *) {
	}
}; 


/**
   \brief functor to wrap statically allocated data a shared pointer representation 

   This functor wraps a statically allocated data into a std::shared_ptr. 
   The data must not be constant and it well not be freed when the shared_ptr is destoyed. 
   The functor is designed to be usable by the std::transform function. 

   \tparam Data the data type to be wrapped into the shared pointer; 
*/
template <typename Data>
class FWrapStaticDataInSharedPointer {
public: 
	typedef std::shared_ptr<Data> PData;
	PData operator () (Data& d) const {
		return PData(&d, void_destructor<Data>());
	}
};


/**
   \ingroup misc
   Function to convert a streamable type from a string to a value. 
   The string may contain whitespaces before and after the value but no other characters. 
   \tparam T some type that supports the >> stream operator 
   \param s the c-string holding the value 
   \param [out] result value of T corresponding to s
   \returns true if s could be parsed sucessfully, and false if not 
 */

template <typename T> 
bool from_string(const char *s, T& result) 
{
	std::istringstream sx(s); 
	sx >> result; 
	if (sx.fail()) 
		return false; 
	if (sx.eof())
		return true; 
	
	std::string remaining; 
	sx >> remaining; 
	bool retval = true; 
	for(auto i = remaining.begin(); i != remaining.end(); ++i) 
		retval &= isspace(remaining[0]); 
	return retval; 

}

/**
   \ingroup misc
   Function to convert a streamable type from a string to a value. 
   The string may contain whitespaces before and after the value but no other characters. 
   \tparam T some type that supports the >> stream operator 
   \param s the c-string holding the value 
   \param [out] result value of T corresponding to s
   \returns true if s could be parsed sucessfully, and false if not 
 */

template <typename T> 
bool from_string(const std::string& s, T& result) 
{
	return from_string(s.c_str(), result); 
}


/**
   \ingroup misc
   Function to convert a streamable type from to a string 
   \tparam T some type that supports the << stream operator 
   \param v the value to be converted 
   \returns string that corresponds to v 
 */

template <typename T> 
const std::string  to_string(typename boost::call_traits<T>::param_type v)
{
	std::stringstream result; 
	result << v; 
	return result.str(); 
}


/*
  The special handling of floating point values is needed, 
  because c++ writes out inf/-inf for infinity, but XML 
  expects uppercase INF/-INF. 
*/
template <typename T> 
const std::string  to_string_fp(T v)
{
	std::stringstream result;
	int inf = std::isinf(v); 
	if (!inf) {
		result << v; 
	} else {
		if (inf < 0) 
			result << "-"; 
		result << "INF"; 
	}
	return result.str(); 
}

template <> 
inline const std::string  to_string<float>(boost::call_traits<float>::param_type v)
{
	return to_string_fp(v); 
}

template <> 
inline const std::string  to_string<double>(boost::call_traits<double>::param_type v)
{
	return to_string_fp(v); 
}


NS_MIA_END

#endif
