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


/**
   @file core/defines.hh 
   This file provides some defines to make life easier 
 */


#ifndef mia_core_defines_hh
#define mia_core_defines_hh

#include <config.h>
#include <gsl++/gsldefines.hh>

/// conveniance define to start the mia namespace 
#define NS_MIA_BEGIN namespace mia {

/// conveniance define to end the mia namespace 
#define NS_MIA_END }

/// conveniance define to use the mia namespace 
#define NS_MIA_USE using namespace mia; 

/// conveniance define to start a namespace 
#define NS_BEGIN(NS) namespace NS {

/// conveniance define to end a namespace 
#define NS_END }

/// conveniance define to use s namespace 
#define NS_USE(NS) using namespace NS


/**
   \brief The main namespace of this library. 
   
   The mia namespace is the general namespace of this library.  
   Currently, only plug-ins have additional namespace to avoid name clashes
   between dynamically loaded modules that might have been developed outside 
   this source package. 
*/
NS_MIA_BEGIN

/// some constants for interpoation types
enum EInterpolation {
	ip_nn,       /**< nearest neighbor interpolation */
	ip_linear,   /**< linear interpolation */
	ip_bspline0, /**< bsplines of degree 0 (nearest neighbor interpolation) */ 
	ip_bspline1, /**< bsplines of degree 1 (linear interpolation)*/ 
	ip_bspline2, /**< bsplines of degree 2 */ 
	ip_bspline3, /**< bsplines of degree 3 */  
	ip_bspline4, /**< bsplines of degree 4 */ 
	ip_bspline5, /**< bsplines of degree 5 */  
	ip_omoms3,   /**< omoms splines of degree 3 */
	ip_unknown   /**< unknown type */
};

/// Enumerate for the types of possible interpolator factories 
enum EInterpolationFactory {
	ipf_spline, /**< bspline based interpolation */ 
	ipf_unknown /**< unkown type */ 
};

NS_MIA_END

/// suffix for plug-ins 
#define MIA_MODULE_SUFFIX "mia"

#ifndef __GNUC__
/// remove the __attribute__ syntax if the compiler is not GCC compatible 
#  define __attribute__(x) 
#endif

#ifdef miacore_EXPORTS
/// Macro to manage Visual C++ style dllimport/dllexport 
#  define EXPORT_CORE DO_EXPORT
#else
/// Macro to manage Visual C++ style dllimport/dllexport 
#  define EXPORT_CORE DO_IMPORT
#endif

/// Macro to set visibility to default, resp. __declspec(dllexport) 
#define EXPORT DO_EXPORT

#ifdef WIN32 
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4251) // class needs to have dll interface to be used ...
#pragma warning(disable: 4231) // nonstandard extension used "extern" before explicit template instanciation
#pragma warning(disable: 4275) // non-dll class used as base 
#pragma warning(disable: 4244) // warning about int->float conversion 
#pragma warning(disable: 4305) // truncation from double to float 
#pragma warning(disable: 4800) // performance warning about forcing value to true or false
#endif
#endif

#ifdef miacore_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM
#  endif
#  define VSTREAM "MIA-CORE"
#endif

#endif

