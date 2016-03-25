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

#include <mia/internal/autotest.hh>
#include <mia/core/vector.hh>

NS_MIA_USE; 


BOOST_AUTO_TEST_CASE (test_init)
{
	double input[6] = {1,2,3,4,5,6}; 

	CDoubleVector v(6, input); 
	
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v[i], input[i]); 
	
}

BOOST_AUTO_TEST_CASE (test_const_init)
{
	const double input[6] = {1,2,3,4,5,6}; 
	const CDoubleVector v(6, input); 
	
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v[i], input[i]); 
	
}

BOOST_AUTO_TEST_CASE (test_new)
{
	double input[6] = {1,2,3,4,5,6}; 

	CDoubleVector v(6); 
	std::copy(input, input + 6, v.begin()); 
	
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v[i], input[i]); 
}

void test_equal(const CDoubleVector& v, const double *test_data) 
{
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v[i], test_data[i]); 
}

BOOST_AUTO_TEST_CASE (test_new2)
{
	double input[6] = {1,2,3,4,5,6}; 

	CDoubleVector v(6); 
	for(int i = 0; i < 6; ++i) {
		v[i] = input[i]; 
	}
	
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v[i], input[i]); 

	const CDoubleVector v2(v); 
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v2[i], input[i]); 
	
	test_equal(v2, input); 

	v = v2; 
	test_equal(v, input); 
}

extern void __pass_by_value(CDoubleVector v, const double *expect, size_t length)
{
	BOOST_CHECK_EQUAL(v.size(), length); 
	test_equal(v, expect); 
}



BOOST_AUTO_TEST_CASE (test_copies)
{
	const double input[6] = {1,2,3,4,5,6}; 

	CDoubleVector v(6, input); 
	test_equal(v, input); 

	CDoubleVector v2(v); 
	
	test_equal(v2, input); 

	CDoubleVector v3(2); 

	v3[0] = 8; 
	v3[1] = 9; 
	BOOST_CHECK_EQUAL(v3[0], 8); 
	BOOST_CHECK_EQUAL(v3[1], 9); 

	__pass_by_value(v, input, 6); 

	v3 = v2; 
	v3.make_unique(); 
	test_equal(v3, input); 
	

}


BOOST_AUTO_TEST_CASE (test_assign_and_pass_by_copy)
{
	CDoubleVector v(6);

	const double expect[6] = {0,1,2,3,4,5}; 
	const double expect_2[6] = {2,3,4,5,6,7}; 
	
	for (int i = 0; i < 6; ++i) 
		v[i] = i; 
	
	__pass_by_value(v, expect, 6); 


	for (int i = 0; i < 6; ++i) 
		v[i] = i + 2; 

	__pass_by_value(v, expect_2, 6); 

}
