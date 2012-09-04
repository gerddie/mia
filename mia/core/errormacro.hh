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

inline void __append_message(std::ostream& PARAM_UNUSED(os))
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


template <typename E, typename... T>
E Except( T ...t )
{
	return E(::mia::__create_message(t...)); 
}


#ifdef NDEBUG
#define mia_assert(cond, msg...)
#else 
#define mia_assert(cond, msg...)					\
	if (!cond) {							\
		std::cerr << "Assertion failed: \'" #cond  << "\' " <<	\
			__FILE__ << ":" << __LINE__ << " "		\
			  << ::mia::__create_message(msg) << "\n";		\
			abort();					\
	}
#endif 


/** 
    \ingroup misc
    \brief  helper macro to create exceptions using stream output for its message 
 */
#define THROW(EXCEPTION, MESSAGE)		\
	do {					\
		std::stringstream msg;		\
		msg << MESSAGE;			\
		throw EXCEPTION(msg.str());	\
	} while (0)

#ifdef NDEBUG

// throw if compiled in release mode but assert if compiled in debug mode
#define DEBUG_ASSERT_RELEASE_THROW(cond, msg... )	\
	if (!cond) {					\
		throw Except<std::logic_error>(msg));	\
	}
#else
#define DEBUG_ASSERT_RELEASE_THROW(cond, msg... )	\
	mia_assert(cond && msg); 
#endif 

NS_MIA_END

#endif
