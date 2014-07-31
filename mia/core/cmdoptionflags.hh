/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_core_cmdoption_flags_hh
#define mia_core_cmdoption_flags_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

enum class CCmdOptionFlags : int {
        none = 0, 
        required = 1, 
        input = 2, 
        output = 4, 
        required_input = 3, 
	required_output = 5,
	validate = 8	
}; 

inline CCmdOptionFlags operator | (CCmdOptionFlags lhs, CCmdOptionFlags rhs) 
{
        return static_cast<CCmdOptionFlags>(  static_cast<int>(lhs) | static_cast<int>(rhs)); 
}

inline CCmdOptionFlags operator  & (CCmdOptionFlags lhs, CCmdOptionFlags rhs) 
{
        return static_cast<CCmdOptionFlags>(  static_cast<int>(lhs) & static_cast<int>(rhs)); 
}

inline CCmdOptionFlags operator -= (CCmdOptionFlags& lhs, CCmdOptionFlags rhs) 
{
        lhs = static_cast<CCmdOptionFlags>(  static_cast<int>(lhs) & ~static_cast<int>(rhs)); 
        return lhs; 
}

inline bool has_flag(CCmdOptionFlags flags, CCmdOptionFlags test) 
{
	return (flags & test) == test; 
}

inline std::ostream& operator << (std::ostream& os, CCmdOptionFlags flags) 
{
        switch (flags) {
        case CCmdOptionFlags::none: os << "CCmdOptionFlags::none"; break; 
        case CCmdOptionFlags::required: os << "CCmdOptionFlags::required"; break; 
        case CCmdOptionFlags::input: os << "CCmdOptionFlags::input"; break; 
        case CCmdOptionFlags::output: os << "CCmdOptionFlags::output"; break; 
        case CCmdOptionFlags::required_input: os << "CCmdOptionFlags::required_input"; break; 
        case CCmdOptionFlags::required_output: os << "CCmdOptionFlags::required_output"; break; 
        case CCmdOptionFlags::validate: os << "CCmdOptionFlags::validate"; break; 

        default: os << "CCmdOptionFlags::<undefined>"; 
        }; 
        return os; 
}

NS_MIA_END

#endif 
