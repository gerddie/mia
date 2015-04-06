/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_internal_pluginsettest_hh
#define mia_internal_pluginsettest_hh

#include <string>
#include <set>
#include <boost/test/unit_test.hpp>

template <typename Hander> 
void test_availabe_plugins(const  Hander& handler, const  std::set<std::string>& test)
{
	auto data = handler.get_set(); 
	BOOST_CHECK_EQUAL(data.size(), test.size()); 
	for (auto p = data.begin(); p != data.end(); ++p) {
		BOOST_CHECK_MESSAGE(test.find(*p) != test.end(), "unexpected plugin '" << *p << "' found"); 
	}
	
	for (auto p = test.begin(); p != test.end(); ++p)
		BOOST_CHECK_MESSAGE(data.find(*p) != data.end(), "expected plugin '" << *p << "' not found"); 

}



#endif 
