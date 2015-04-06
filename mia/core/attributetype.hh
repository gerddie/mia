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

#ifndef mia_core_attributetype_hh
#define mia_core_attributetype_hh

#include <vector>
#include <mia/core/defines.hh>
#include <miaconfig.h>


NS_MIA_BEGIN

struct EAttributeType {
        static const int attr_unknown =    0; 
        static const int attr_bool    =    1; 
        static const int attr_uchar   =    2; 
        static const int attr_schar   =    3; 
        static const int attr_ushort  =    4; 
        static const int attr_sshort  =    5; 
        static const int attr_uint    =    6; 
        static const int attr_sint    =    7; 
        static const int attr_ulong   =    8; 
        static const int attr_slong   =    9; 
        static const int attr_float   =   10; 
        static const int attr_double  =   11; 
        static const int attr_string  =   12; 

        static const int vector_bit = 0x80000000; 

        static int scalar_type(int type) {
                return type & scalar_mask; 
        }; 

        static bool is_vector(int type) {
                return type & vector_bit; 
        }

private: 
        static const int scalar_mask  =0x7FFFFFFF; 

}; 

template <typename T> 
struct attribute_type : public EAttributeType { 
        static const int value = EAttributeType::attr_unknown;
}; 

#define ATTR_TYPEID(T, ID)                                      \
        template <>                                             \
        struct attribute_type<T> : public EAttributeType{       \
                static const int value = ID;  \
        };

ATTR_TYPEID(bool, attr_bool); 
ATTR_TYPEID(unsigned char, attr_uchar); 
ATTR_TYPEID(signed char, attr_schar); 

ATTR_TYPEID(unsigned short, attr_ushort); 
ATTR_TYPEID(signed short, attr_sshort); 

ATTR_TYPEID(unsigned int, attr_uint); 
ATTR_TYPEID(signed int, attr_sint); 

#ifdef LONG_64BIT
ATTR_TYPEID(unsigned long, attr_ulong); 
ATTR_TYPEID(signed long, attr_slong); 
#endif

ATTR_TYPEID(float, attr_float); 
ATTR_TYPEID(double, attr_double); 

ATTR_TYPEID(std::string, attr_string); 

#undef ATTR_TYPEID

template <typename T> 
struct attribute_type<std::vector<T>> : public EAttributeType{
        static const int value = attribute_type<T>::value | EAttributeType::vector_bit;
}; 

NS_MIA_END
        
#endif 
