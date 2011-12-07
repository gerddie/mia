/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#ifndef __MIA_TOOLS_HH
#define __MIA_TOOLS_HH 1

#include <list>
#include <limits>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <mia/core/defines.hh>


NS_MIA_BEGIN

/** \ingroup helpers 
    \brief A Scope based helper class to save and restore the current working directory 

   A helper class that stores the current working directory on construction
   and goes back to the it when the class intance is destroyed. 
*/
class EXPORT_CORE CCWDSaver {
	char *cwd;
public:
	CCWDSaver();
	~CCWDSaver();
};

#ifndef WIN32
/*! \ingroup functors   
  \brief a functor to search  for files 
*/
class  FSearchFiles  {
	std::list<std::string>& result;
	const std::string pattern;
public:
	/** constructor of functor
	    \param __result takes an (empty) list to which the found files will be stored
	    \param __pattern holds the search pattern
	*/
	FSearchFiles(std::list<std::string>& __result, const std::string& __pattern);

	/** search the given \a path with the stored search pattern and add the found files to result */
	void operator()(const std::string& path);
};
#endif


#ifndef _GNU_SOURCE
/**
   \ingroup helpers 
   Provide sincosf conveniance functions for sin and cos if the GNU GCC extension is not available. 
 */
void sincosf(float x, float *sin, float *cos); 

/**
   \ingroup helpers 
   Provide sincosf conveniance functions for sin and cos if the GNU GCC extension is not available. 
 */
void sincos(double x, double *sin, double *cos); 
#endif

template <typename T, bool is_float> 
struct __round {

	static T apply(double x) {
		return x; 
	}
}; 

template <typename T> 
struct __round<T, false> {
	static T apply(double x) {
		return static_cast<T>(rint(x));
	}
};


/**
   \ingroup helpers 
   
   A simple class to round floating point numbers onyl if necessary. 
   If the target is a floating point values then the result is just passed through, 
   otherwise rint is used for rounding 
   \param x 
   \returns rounded value or x
*/
template <typename T> 
T mia_round(double x) 
{
	const bool is_floating_point = std::is_floating_point<T>::value; 
	return __round<T, is_floating_point>::apply(x); 
}



template <typename T, bool is_float> 
struct __round_clamped {

	static T apply(double x) {
		return x; 
	}
}; 

template <> 
struct __round_clamped<float, true> {

	static float apply(double x) {
		double y =  x < std::numeric_limits<float>::max() ? 
			( x > -std::numeric_limits<float>::max() ? x : -std::numeric_limits<float>::max()) : 
			std::numeric_limits<float>::max(); 
		return static_cast<float>(y); 
	}
}; 

template <> 
struct __round_clamped<bool, false> {
	static float apply(double x) {
		return x > 0.5; 
	}
}; 


template <typename T> 
struct __round_clamped<T, false> {
	static T apply(double x) {
		const double y = rint(x); 
		const double yy = y < std::numeric_limits<T>::max() ? 
			( y > std::numeric_limits<T>::min() ? y : std::numeric_limits<T>::min()) :
			std::numeric_limits<T>::max(); 
		return static_cast<T>(yy);
	}
};

/**
   \ingroup helpers 
   
   A simple class to round floating point numbers onyl if necessary. 
   If the target is a floating point values then the result is just passed through, 
   otherwise rint is used for rounding 
   \param x 
   \returns rounded value or x
*/
template <typename T> 
T mia_round_clamped(double x) 
{
	const bool is_floating_point = std::is_floating_point<T>::value; 
	return __round_clamped<T, is_floating_point>::apply(x); 
}


NS_MIA_END

#endif
