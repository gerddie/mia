/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011 Gert Wollny 
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef mia_gsldefines_hh
#define mia_gsldefines_hh

#if defined(_MSC_VER)
#  define DO_EXPORT __declspec(dllexport) 
#  define DO_IMPORT __declspec(dllimport) 
#elif  defined(__GNUC__)
#  define DO_EXPORT __attribute__((visibility("default")))
#  define DO_IMPORT __attribute__((visibility("default")))	
#else
#  define DO_EXPORT
#  define DO_IMPORT
#endif

#ifdef miagsl_EXPORTS
#define EXPORT_GSL DO_EXPORT
#else
#define EXPORT_GSL DO_IMPORT
#endif


#endif
