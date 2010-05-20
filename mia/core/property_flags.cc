/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <mia/core/property_flags.hh>

NS_MIA_BEGIN

const char *property_gradient = "gradient";

bool CPropertyFlagHolder::has(const char *property) const
{
	return _M_properties.find(property) != _M_properties.end(); 
}

void CPropertyFlagHolder::add(const char *property)
{
	_M_properties.insert(property); 
}

bool CPropertyFlagHolder::has_all_in(const CPropertyFlagHolder& testset) const
{
	for(std::set<const char *>::const_iterator ti = testset._M_properties.begin(); 
	    ti != testset._M_properties.end(); ++ti) {
		if (!has(*ti))
			return false; 
	}
	return true; 
}

NS_MIA_END

