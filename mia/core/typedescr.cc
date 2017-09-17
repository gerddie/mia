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

#include <mia/core/typedescr.hh>

NS_MIA_BEGIN

const char *timestep_type::type_descr = "timestep";


DEFINE_TYPE_DESCR2(int16_t, "short"); 
DEFINE_TYPE_DESCR2(int32_t, "int"); 
DEFINE_TYPE_DESCR2(int64_t, "long"); 
DEFINE_TYPE_DESCR(float); 
DEFINE_TYPE_DESCR(double); 
DEFINE_TYPE_DESCR(bool); 

DEFINE_TYPE_DESCR2(int8_t, "sbyte"); 
DEFINE_TYPE_DESCR2(uint8_t, "ubyte"); 

DEFINE_TYPE_DESCR2(uint16_t, "ushort"); 
DEFINE_TYPE_DESCR2(uint32_t, "uint"); 
DEFINE_TYPE_DESCR2(uint64_t, "ulong");
DEFINE_TYPE_DESCR2(std::string, "string");

DEFINE_TYPE_DESCR2(std::vector<int16_t>, "vshort"); 
DEFINE_TYPE_DESCR2(std::vector<int32_t>, "vint"); 
DEFINE_TYPE_DESCR2(std::vector<int64_t>, "vlong"); 
DEFINE_TYPE_DESCR2(std::vector<uint32_t>, "vuint");
DEFINE_TYPE_DESCR2(std::vector<uint16_t>,  "vushort");
DEFINE_TYPE_DESCR2(std::vector<uint64_t>, "vulong");
DEFINE_TYPE_DESCR2(std::vector<float>, "vfloat"); 
DEFINE_TYPE_DESCR2(std::vector<double>, "vdouble"); 
DEFINE_TYPE_DESCR2(std::vector<bool>, "vbool"); 
DEFINE_TYPE_DESCR2(std::vector<std::string>, "vstring"); 


NS_MIA_END
