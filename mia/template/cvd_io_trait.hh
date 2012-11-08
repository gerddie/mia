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

#ifndef mia_template_cvd_io_trait_hh
#define mia_template_cvd_io_trait_hh

#include <mia/core/defines.hh>
#include <istream>
#include <ostream> 

NS_MIA_BEGIN

/**
   \ingroup traits
   \brief Structure to read and write ND vectors to and from cvd files
 */
template <typename T> 
struct NDVectorIOcvd {
	static bool read(std::istream& is, T& value) {
		static_assert(sizeof(T) == 0, "NDVectorIOcvd::read needs specialization"); 
		return false; 
	}
	static void write(std::ostream& os, const T& value){
		static_assert(sizeof(T) == 0, "NDVectorIOcvd::read needs specialization"); 
	}
}; 
NS_MIA_END

#endif 
