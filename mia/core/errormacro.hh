/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_errormacro_hh
#define mia_core_errormacro_hh


#include <iostream>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include <mia/core/defines.hh>


NS_MIA_BEGIN

template <typename V>
void __append_message(std::ostream& os, const V& v)
{
	os << v; 
}

inline void __append_message(std::ostream& MIA_PARAM_UNUSED(os))
{
}

template <typename V, typename... T>
void __append_message(std::ostream& os, const V& v, T ...t)
{
	os << v; 
	::mia::__append_message(os, t...); 
}

template <typename... T>
const std::string __create_message(T ...t)
{
	std::stringstream msg; 
	::mia::__append_message(msg, t...); 
	return msg.str(); 
}

/**
    \ingroup misc
    \brief  helper template to create exceptions with complex messages
    
    This function template creates an exception of the given type. The 
    exception message is created by feeding the arguments from left to right 
    into a stringstream. An abitrary number of arguments can be given, but they all 
    must support the operator << for writing to an output stream 
    
    \tparam E the exception to be created 
    \tparam T... the variadic list of arguments 
    \param ...t the arguments used to create the exception message
*/
template <typename E, typename... T>
E create_exception( T ...t )
{
	return E(::mia::__create_message(t...)); 
}

#ifdef NDEBUG

/** \brief throw if compiled in release mode but assert if compiled in debug mode
    This macro can be used to get an assertion in a debug build and an exeption 
    in release mode. 

    \param cond the condition that is tested in the assertion 
    \param msg a varaidic list of parameters that is used to create the assertion/exception message
 */ 
#define DEBUG_ASSERT_RELEASE_THROW(cond, msg... )	\
	if (!(cond)) {					\
		throw ::mia::create_exception<std::logic_error>(msg);	\
	}
#else
#define DEBUG_ASSERT_RELEASE_THROW(cond, msg... )			\
	if (!(cond)) {							\
		std::cerr << "Assertion failed: \'" #cond  << "\' " <<	\
			__FILE__ << ":" << __LINE__ << " "		\
			  << ::mia::__create_message(msg) << "\n";	\
			abort();					\
	}
#endif 

NS_MIA_END

#endif
