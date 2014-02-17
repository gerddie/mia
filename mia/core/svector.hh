/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <istream>
#include <ostream>
#include <vector>
#include <stdexcept>

#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

template <typename T> 
std::ostream&  operator << (std::ostream& os, const std::vector<T>& v) 
{
	for(auto x: v)
		os << x << ","; 
	return os; 
}

template <typename T> 
std::istream&  operator >> (std::istream& is, std::vector<T>& v)
{
	vector<T> values; 
	T token; 
	while(std::getline(is, token, ','))
		values.push_back(token); 
		
	if (!v.empty() && v.size() != values.size()) {
			throw create_exception<invalid_argument>("Reading vector: expected ", 
								 v.size(), " values, but got ", values.size()); 
	}
	v.swap(values); 
	return is; 
}


NS_MIA_END
