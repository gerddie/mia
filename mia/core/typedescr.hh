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


#ifndef mia_core_model_hh
#define mia_core_model_hh


#include <mia/core/defines.hh>
#include <string>
#include <vector>

NS_MIA_BEGIN

/**
   \ingroup helpers 
   Template type parameter for time-step plugins 
 */
struct EXPORT_CORE timestep_type {
	/// value for plug-in search path component 
	static const char *type_descr;
};


/*
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
	

DECLARE_TYPE_DESCR(signed char); 
DECLARE_TYPE_DESCR(unsigned char); 
DECLARE_TYPE_DESCR(short); 
DECLARE_TYPE_DESCR(int); 
DECLARE_TYPE_DESCR(long); 
DECLARE_TYPE_DESCR(unsigned int);
DECLARE_TYPE_DESCR(unsigned short);
DECLARE_TYPE_DESCR(unsigned long);
DECLARE_TYPE_DESCR(float); 
DECLARE_TYPE_DESCR(double); 
DECLARE_TYPE_DESCR(bool); 
DECLARE_TYPE_DESCR(std::string); 


DECLARE_TYPE_DESCR(std::vector<short>); 
DECLARE_TYPE_DESCR(std::vector<int>); 
DECLARE_TYPE_DESCR(std::vector<long>); 
DECLARE_TYPE_DESCR(std::vector<unsigned int>);
DECLARE_TYPE_DESCR(std::vector<unsigned short>);
DECLARE_TYPE_DESCR(std::vector<unsigned long>);
DECLARE_TYPE_DESCR(std::vector<float>); 
DECLARE_TYPE_DESCR(std::vector<double>); 
DECLARE_TYPE_DESCR(std::vector<bool>); 
DECLARE_TYPE_DESCR(std::vector<std::string>); 

NS_MIA_END

#endif
