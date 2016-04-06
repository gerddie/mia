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


#include <sstream>
#include <vector>

#include <mia/internal/autotest.hh>
#include <mia/core/svector.hh>

using namespace std; 
using namespace mia; 

BOOST_AUTO_TEST_CASE( test_read_strings_to_empty_vector ) 
{
	istringstream is("test1,test2,test3"); 
	
	vector<string> result; 
	is >> result; 

	BOOST_REQUIRE(result.size() == 3u); 
	
	BOOST_CHECK_EQUAL(result[0], "test1"); 
	BOOST_CHECK_EQUAL(result[1], "test2"); 
	BOOST_CHECK_EQUAL(result[2], "test3");
}

BOOST_AUTO_TEST_CASE( test_read_strings_to_presized_vector ) 
{
	istringstream is("test1,test2,test3"); 
	
	vector<string> result(3); 
	is >> result; 

	BOOST_REQUIRE(result.size() == 3u); 
	BOOST_CHECK_EQUAL(result[0], "test1"); 
	BOOST_CHECK_EQUAL(result[1], "test2"); 
	BOOST_CHECK_EQUAL(result[2], "test3");

	
}

BOOST_AUTO_TEST_CASE( test_read_strings_to_presized_vector_too_many ) 
{
	istringstream is("test1,test2,test3"); 
	
	vector<string> result(2); 
	BOOST_CHECK_THROW(is >> result, invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_read_strings_to_presized_vector_throw_not_enough ) 
{
	istringstream is("test1,test2,test3"); 
	
	vector<string> result(4); 
	BOOST_CHECK_THROW(is >> result, invalid_argument); 
}

BOOST_AUTO_TEST_CASE( test_read_float ) 
{
	istringstream is("1.0,1e-10,3.0"); 
	
	vector<float> result; 
	is >> result; 

	BOOST_REQUIRE(result.size() == 3u); 
	BOOST_CHECK_EQUAL(result[0], 1.0f); 
	BOOST_CHECK_EQUAL(result[1], 1e-10f); 
	BOOST_CHECK_EQUAL(result[2], 3.0f);
	
}

BOOST_AUTO_TEST_CASE( test_read_float_fail ) 
{
	istringstream is("1.0f,1e-10,3.0"); 
	
	vector<float> result; 
	BOOST_CHECK_THROW(is >> result, invalid_argument); 
}




