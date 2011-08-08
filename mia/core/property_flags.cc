/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#include <mia/core/property_flags.hh>

NS_MIA_BEGIN

EXPORT_CORE const char *property_gradient = "gradient";

CPropertyFlagHolder::~CPropertyFlagHolder()
{
}

bool CPropertyFlagHolder::has(const char *property) const
{
	if (m_properties.find(property) != m_properties.end())
		return true; 
	return do_has(property); 
}

void CPropertyFlagHolder::add(const char *property)
{
	m_properties.insert(property);
}

bool CPropertyFlagHolder::has_all_in(const CPropertyFlagHolder& testset) const
{
	for(auto ti = testset.m_properties.begin();
	    ti != testset.m_properties.end(); ++ti) {
		
		if (!has(*ti))
			return false;
	}
	return true;
}

CPropertyFlagHolder::Set
CPropertyFlagHolder::get_missing_properties(const CPropertyFlagHolder& testset)const
{
	Set  result; 
	for(auto ti = testset.m_properties.begin(); ti != testset.m_properties.end(); ++ti) {
		if (!has(*ti))
			result.insert(*ti); 
	}
	return result;
}

bool CPropertyFlagHolder::do_has(const char */* property*/) const
{
	// placeholder, derived classes may run some additional tests
	return false; 
}

NS_MIA_END

