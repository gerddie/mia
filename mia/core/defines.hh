/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifndef mia_core_defines_hh
#define mia_core_defines_hh

#define NS_MIA_BEGIN namespace mia {
#define NS_MIA_END }
#define NS_MIA_USE using namespace mia; 

#define NS_BEGIN(NS) namespace NS {
#define NS_END }
#define NS_USE(NS) using namespace NS

NS_BEGIN(mia)

/// some constants for interpoation types
enum EInterpolation {
	ip_nn,       /**< nearest neighbor interpolation */
	ip_linear,   /**< linear interpolation */
	ip_bspline2, /**< bsplines of degree 2 */ 
	ip_bspline3, /**< bsplines of degree 3 */  
	ip_bspline4, /**< bsplines of degree 4 */ 
	ip_bspline5, /**< bsplines of degree 5 */  
	ip_omoms3,    /**< omoms splines of degree 3 */
	ip_unknown
};
NS_END

#ifndef __GNUC__
#define __attribute__(x) 
#endif


#ifdef WIN32 
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define EXPORT __declspec(dllexport) 

#ifdef miacore_EXPORTS
#define EXPORT_CORE __declspec(dllexport) 
#else
#define EXPORT_CORE __declspec(dllimport) 
#endif

#ifdef _MSC_VER
x1
#pragma warning(disable: 4251) // class needs to have dll interface to be used ...
#pragma warning(disable: 4231) // nonstandard extension used "extern" before explicit template instanciation
#pragma warning(disable: 4275) // non-dll class used as base 
#pragma warning(disable: 4244) // warning about int->float conversion 
#pragma warning(disable: 4305) // truncation from double to float 
#pragma warning(disable: 4800) // performance warning about forcing value to true or false

#define G_MODULE_SUFFIX "dll"
#endif

#else

#  ifdef __GNUC__
#    define EXPORT_CORE __attribute__((visibility("default")))
#    define EXPORT __attribute__((visibility("default")))
#  else
#    define EXPORT_CORE
#    define EXPORT __attribute__((visibility("default")))
#  endif

#define G_MODULE_SUFFIX "so"

#endif

#ifdef miacore_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM
#  endif
#  define VSTREAM "MIA-CORE"
#endif

#endif

