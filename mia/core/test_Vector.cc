/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

	CDoubleVector v(6, input); 
	
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

void test_cost(const CDoubleVector& v, const double *test_data) 
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

	CDoubleVector v2(v); 
	for(int i = 0; i < 6; ++i) 
		BOOST_CHECK_EQUAL(v2[i], input[i]); 
	
	test_cost(v2, input); 

	v = v2; 
	test_cost(v, input); 
}

