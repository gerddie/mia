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

#ifndef mia_core_typedescr_hh
#define mia_core_typedescr_hh


#include <mia/core/defines.hh>
#include <string>
#include <vector>

NS_MIA_BEGIN

/// @cond INTERNAL 

/**
   \ingroup misc
   Template type parameter for time-step plugins 
 */
struct EXPORT_CORE timestep_type {
	/// value for plug-in search path component 
	static const char *type_descr;
};

/**
   \ingroup traits 
   \brief Type description template
   This template is used to create a type description for command line and 
   plug-in parameters. It needs to be specialized for each type that is used 
   in the command line parser or spline parameter parser 
   \tparam T the type to be described 
*/
template <typename T> 
struct __type_descr {
	static const char * const value; 
}; 

/**
   A macro to make the declaration of the __type_descr specialization 
 */
#define DECLARE_TYPE_DESCR(type)			\
	template <>				\
	struct EXPORT_CORE __type_descr<type> {		\
		static const char * const value; \
	}
/**
   A macro to define the value of the  of the __type_descr specialization directly 
   based on the type name 
   \param type the type to be specialized 
*/
#define DEFINE_TYPE_DESCR(type) const char * const __type_descr<type>::value = #type;

/**
   A macro to define the value of the  of the __type_descr specialization that takes 
   a special name
   \param type the type to be specialized 
   \param name string to represent the type 
 */
#define DEFINE_TYPE_DESCR2(type, name) const char * const __type_descr<type>::value = name;


/// @endcond 	

/// @cond NEVER 

DECLARE_TYPE_DESCR(int8_t); 
DECLARE_TYPE_DESCR(uint8_t); 
DECLARE_TYPE_DESCR(int16_t); 
DECLARE_TYPE_DESCR(int32_t); 
DECLARE_TYPE_DESCR(int64_t); 
DECLARE_TYPE_DESCR(uint16_t);
DECLARE_TYPE_DESCR(uint32_t);
DECLARE_TYPE_DESCR(uint64_t);
DECLARE_TYPE_DESCR(float); 
DECLARE_TYPE_DESCR(double); 
DECLARE_TYPE_DESCR(bool); 
DECLARE_TYPE_DESCR(std::string); 


DECLARE_TYPE_DESCR(std::vector<int16_t>); 
DECLARE_TYPE_DESCR(std::vector<int32_t>); 
DECLARE_TYPE_DESCR(std::vector<int64_t>); 
DECLARE_TYPE_DESCR(std::vector<uint32_t>);
DECLARE_TYPE_DESCR(std::vector<uint16_t>);
DECLARE_TYPE_DESCR(std::vector<uint64_t>);
DECLARE_TYPE_DESCR(std::vector<float>); 
DECLARE_TYPE_DESCR(std::vector<double>); 
DECLARE_TYPE_DESCR(std::vector<bool>); 
DECLARE_TYPE_DESCR(std::vector<std::string>); 

/// @endcond 

NS_MIA_END

#endif
