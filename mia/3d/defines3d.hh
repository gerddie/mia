/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_3d_defines_hh
#define mia_3d_defines_hh

#ifdef WIN32
#  include <cstdlib>
   inline double drand48() {
	return double(rand()) / RAND_MAX; 
   }
#endif

#ifdef WIN32
#  ifdef mia3d_EXPORTS
#    define EXPORT_3D __declspec(dllexport) 
#  else
#    define EXPORT_3D __declspec(dllimport) 
#  endif
#  ifdef mia3dtest_EXPORTS
#    define EXPORT_3DTEST __declspec(dllexport) 
#  else
#    define EXPORT_3DTEST __declspec(dllimport) 
#  endif
#else
#  ifdef __GNUC__
#    define EXPORT_3D __attribute__((visibility("default")))
#    define EXPORT_3DTEST __attribute__((visibility("default")))
#  else
#    define EXPORT_3D 
#    define EXPORT_3DTEST 
#  endif
#endif

#ifdef mia3d_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM  
#  endif 
#  define VSTREAM "MIA3D"
#endif

#ifdef mia3dtest_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM  
#  endif 
#  define VSTREAM "MIA3DTEST"
#endif


#endif
