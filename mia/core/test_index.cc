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

#define BOOST_TEST_MODULE CIRCLEINDEX

#include <mia/internal/autotest.hh>
#include <mia/core/index.hh>

NS_MIA_USE

BOOST_AUTO_TEST_CASE ( test_index_basics ) 
{
	CCircularIndex idx(4, 0); 

	BOOST_CHECK_EQUAL(idx.next(), 0u); 
	BOOST_CHECK_EQUAL(idx.fill(),0u); 
	
	BOOST_CHECK_EQUAL(idx.value(0), 0u); 
	BOOST_CHECK_EQUAL(idx.value(1), 1u); 
	BOOST_CHECK_EQUAL(idx.value(2), 2u); 
	BOOST_CHECK_EQUAL(idx.value(3), 3u); 

	idx.insert_one(); 
	BOOST_CHECK_EQUAL(idx.next(), 1u);
	BOOST_CHECK_EQUAL(idx.fill(), 1u); 
	
	idx.insert_one(); 
	BOOST_CHECK_EQUAL(idx.next(), 2u);
	BOOST_CHECK_EQUAL(idx.fill(), 2u); 

	idx.insert_one(); 
	BOOST_CHECK_EQUAL(idx.next(), 3u);

	idx.insert_one(); 
	BOOST_CHECK_EQUAL(idx.next(), 0u);
	
	idx.new_start(0); 
	BOOST_CHECK_EQUAL(idx.value(0), 0u); 
	BOOST_CHECK_EQUAL(idx.value(1), 1u); 
	BOOST_CHECK_EQUAL(idx.value(2), 2u); 
	BOOST_CHECK_EQUAL(idx.value(3), 3u); 

	idx.new_start(1); 
	BOOST_CHECK_EQUAL(idx.fill(), 3u); 
	BOOST_CHECK_EQUAL(idx.value(0), 1u); 
	BOOST_CHECK_EQUAL(idx.value(1), 2u); 
	BOOST_CHECK_EQUAL(idx.value(2), 3u); 
	BOOST_CHECK_EQUAL(idx.value(3), 0u); 
	
	idx.insert_one(); 
	BOOST_CHECK_EQUAL(idx.fill(), 4u); 
	BOOST_CHECK_EQUAL(idx.next(), 1u);

	idx.new_start(2);
	BOOST_CHECK_EQUAL(idx.value(0), 2u); 
	BOOST_CHECK_EQUAL(idx.value(1), 3u); 
	BOOST_CHECK_EQUAL(idx.value(2), 0u); 
	BOOST_CHECK_EQUAL(idx.value(3), 1u); 

}
