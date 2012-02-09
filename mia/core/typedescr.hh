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

template <typename T> 
struct __type_descr {
	static const char * const name; 
}; 

#define DEFINE_TYPE_DESCR(type)			\
	template <>				\
	struct __type_descr<type> {		\
		static const char * const name; \
	}

#define DECLARE_TYPE_DESCR(type) const char * const __type_descr<type>::name = #type;
#define DECLARE_TYPE_DESCR2(type, n) const char * const __type_descr<type>::name = n;
	


DEFINE_TYPE_DESCR(short); 
DEFINE_TYPE_DESCR(int); 
DEFINE_TYPE_DESCR(long); 
DEFINE_TYPE_DESCR(unsigned int);
DEFINE_TYPE_DESCR(unsigned short);
DEFINE_TYPE_DESCR(unsigned long);
DEFINE_TYPE_DESCR(float); 
DEFINE_TYPE_DESCR(double); 
DEFINE_TYPE_DESCR(bool); 
DEFINE_TYPE_DESCR(std::string); 


DEFINE_TYPE_DESCR(std::vector<short>); 
DEFINE_TYPE_DESCR(std::vector<int>); 
DEFINE_TYPE_DESCR(std::vector<long>); 
DEFINE_TYPE_DESCR(std::vector<unsigned int>);
DEFINE_TYPE_DESCR(std::vector<unsigned short>);
DEFINE_TYPE_DESCR(std::vector<unsigned long>);
DEFINE_TYPE_DESCR(std::vector<float>); 
DEFINE_TYPE_DESCR(std::vector<double>); 
DEFINE_TYPE_DESCR(std::vector<bool>); 
DEFINE_TYPE_DESCR(std::vector<std::string>); 

NS_MIA_END

#endif
