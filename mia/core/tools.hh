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

// helper functions for conversion from and to strings 

#ifndef mia_core_tools_hh
#define mia_core_tools_hh

#include <sstream>
#include <string>
#include <boost/call_traits.hpp>
#include <mia/core/defines.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

/**
   \ingroup helpers
   Function to convert a streamable type from a string to a value. 
   The string may contain whitespaces before and after the value but no other characters. 
   \tparam T some type that supports the >> stream operator 
   \param s the c-string holding the value 
   \param [out] value of T corresponding to s
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
   \ingroup helpers
   Function to convert a streamable type from a string to a value. 
   The string may contain whitespaces before and after the value but no other characters. 
   \tparam T some type that supports the >> stream operator 
   \param s the c-string holding the value 
   \param [out] value of T corresponding to s
   \returns true if s could be parsed sucessfully, and false if not 
 */

template <typename T> 
bool from_string(const std::string& s, T& result) 
{
	return from_string(s.c_str(), result); 
}


/**
   \ingroup helpers
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

NS_MIA_END

#endif
