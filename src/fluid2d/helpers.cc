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

#include "helpers.hh"

int32 log2(uint32 in)
{
       int32 res = -1;

       while (in) {
              res++;
              in >>= 1;
       }

       return res;
}

uint32 exp2(int32 in)
{
       if (in < 0)
              return 0;

       uint32 res = 1;

       while (in--) {
              res <<= 1;
       }

       return res;
}

/* CVS LOG

   $Log: helpers.cc,v $
   Revision 1.3  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

