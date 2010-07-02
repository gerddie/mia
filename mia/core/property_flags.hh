/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_core_property_flags_hh
#define mia_core_property_flags_hh

#include <set>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

extern const char *property_gradient;

class CPropertyFlagHolder {
public:
	bool has(const char *property) const;
	void add(const char *property);

	bool has_all_in(const CPropertyFlagHolder& testset)const;
private:
	std::set<const char *> _M_properties;
};


NS_MIA_END
#endif
