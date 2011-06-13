/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2011
 * BIT, ETSI Telecomunicacion, UPM
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

#ifndef mia_core_labelmap_hh
#define mia_core_labelmap_hh


#include <iosfwd>
#include <map>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \brief A simple class to add loadind and storeing to a map of labels. 
*/
class CLabelMap: public std::map<unsigned short,  unsigned short> {
public: 
	CLabelMap() = default; 

	/// standard copy construtor 
	CLabelMap(const CLabelMap& org) = default;
	
	/**
	   Constructor to load the map from a file
	   \param is input stream 
	 */
	CLabelMap(std::istream& is); 

	/**
	   Function to save the map to a file 
	   \param os  output stream 
	   \returns true if successfull and false otherwise 
	 */
	void save(std::ostream& is); 
};

NS_MIA_END

#endif 
