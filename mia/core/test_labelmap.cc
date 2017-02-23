/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <sstream> 
#include <mia/internal/autotest.hh>
#include <mia/core/labelmap.hh>

NS_MIA_USE

using std::istringstream; 
using std::ostringstream; 


BOOST_AUTO_TEST_CASE ( test_labelmap ) 
{
	CLabelMap map; 

	map[2] = 3; 
	map[4] = 2; 
	map[9] = 7; 

	ostringstream os; 
	map.save(os); 

	istringstream is(os.str()); 

	CLabelMap new_map(is); 
	
	BOOST_CHECK_EQUAL(new_map.size(), map.size()); 

	BOOST_CHECK_EQUAL(map[2], 3); 
	BOOST_CHECK_EQUAL(map[4], 2); 
	BOOST_CHECK_EQUAL(map[9], 7); 
}


BOOST_AUTO_TEST_CASE ( test_labelmap_fail_1 ) 
{
	istringstream test_false("nothing real"); 
	BOOST_CHECK_THROW(CLabelMap test(test_false), std::invalid_argument); 
}

BOOST_AUTO_TEST_CASE ( test_labelmap_fail_2 ) 
{
	istringstream test_false("MiaLabelmap\n 2\n 3 1"); 
	BOOST_CHECK_THROW(CLabelMap test(test_false), std::invalid_argument); 
}

BOOST_AUTO_TEST_CASE ( test_labelmap_fail_3 ) 
{
	istringstream test_false("MiaLabelmap\n 2\n 3 1\n 4"); 
	BOOST_CHECK_THROW(CLabelMap test(test_false), std::invalid_argument); 
}
