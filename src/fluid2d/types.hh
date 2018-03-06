/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef __types_h
#define __types_h

#if HAVE_CONFIG_H
#include <config.h>
#endif

typedef unsigned char uint8;
typedef signed char int8;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned int uint32;
typedef int int32;


#ifdef WORDS_BIGENDIAN
inline void ENDIANDAPT(int16& x)
{
       x = ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}
inline void ENDIANADAPT(uint16& x)
{
       x = ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}
#else
#define ENDIANADAPT(x)
#endif


#endif

/* CVS LOG

   $Log: types.hh,v $
   Revision 1.2  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

