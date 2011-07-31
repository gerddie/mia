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

#include <mia/core/labelmap.hh>
#include <mia/core/errormacro.hh>
#include <string>
#include <stdexcept>
#include <ostream>
#include <istream>

NS_MIA_BEGIN

using std::string; 
using std::invalid_argument; 

CLabelMap::CLabelMap(std::istream& is)
{
	string header; 
	is >> header; 
	if (header != "MiaLabemap") 
		THROW(invalid_argument, 
		      "C2DLabelMapImageFilterFactory: input does not contain a label map"); 
	
	int n; 
	is >> n; 
	int idx; 
	int new_label; 
	while (is.good() && n--) {
		is >> idx >> new_label; 
		(*this)[idx] = new_label; 
	}
	if (is.fail()) 
		THROW(invalid_argument, "C2DLabelMapImageFilterFactory: bogus label map"); 
	
}

void CLabelMap::save(std::ostream& os)
{
	os << "MiaLabemap\n" << size() << "\n"; 
	for (auto i = begin(); i != end() && os.good(); ++i) 
		os << i->first << " " << i->second << "\n"; 
}

NS_MIA_END
