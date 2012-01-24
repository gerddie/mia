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


#include <mia/core/typedescr.hh>

NS_MIA_BEGIN

const char *timestep_type::type_descr = "timestep";

DECLARE_TYPE_DESCR(short); 
DECLARE_TYPE_DESCR(int); 
DECLARE_TYPE_DESCR(long); 
DECLARE_TYPE_DESCR(float); 
DECLARE_TYPE_DESCR(double); 
DECLARE_TYPE_DESCR(bool); 

DECLARE_TYPE_DESCR2(unsigned short, "ushort"); 
DECLARE_TYPE_DESCR2(unsigned int, "uint"); 
DECLARE_TYPE_DESCR2(unsigned long, "ulong");
DECLARE_TYPE_DESCR2(std::string, "string");

NS_MIA_END
