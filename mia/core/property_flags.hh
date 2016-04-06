/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_core_property_flags_hh
#define mia_core_property_flags_hh

#include <set>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/// constant defining the gradient property 
extern EXPORT_CORE const char *property_gradient;


/**
   \ingroup cmdline
   \brief This class holds a set of properties. 
   
   This class holds a set of properies that can be compared 
   \remark These properties are stored as plain text 
   strings, but in the end only the pointer is compared. 
*/
class  EXPORT_CORE CPropertyFlagHolder {
public:
	/// define the type of the set of flags 
	typedef std::set<const char *> Set;

	/**
	   Ensure evirtual destructor 
	 */
	virtual ~CPropertyFlagHolder(); 
	/** check if a certain property is available 
	    \param property
	    \returns true if the property is available
	*/
	bool has(const char *property) const;

	/** add  a certain property
	    \param property proprety to add 
	*/
	void add(const char *property);

	/**
	   Tests if all properties in the testset are available in this instance 
	   \param testset
	 */
	bool has_all_in(const CPropertyFlagHolder& testset)const;

	/**
	   Evalaute the set of properties that is availabe in the testset but not in this one
	   @param testset 
	   @return set of missing flags 
	 */
	Set get_missing_properties(const CPropertyFlagHolder& testset)const; 
private:
	virtual bool do_has(const char *property) const;
	Set m_properties;
};

NS_MIA_END
#endif
