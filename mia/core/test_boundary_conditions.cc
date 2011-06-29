/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#include <mia/core/boundary_conditions.hh>

BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_needed ) 
{
	std::vector<int> index  = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; 
	std::vector<int> result = { 1, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 1 }; 
	std::vector<double> weights(12); 
	
	CMirrorOnBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}

BOOST_AUTO_TEST_CASE( test_mirror_on_boundary_notneeded ) 
{
	std::vector<int> index  = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	std::vector<int> result = {0, 1, 2, 3, 4, 5, 6, 7, 8 }; 
	std::vector<double> weights(9); 
	
	CMirrorOnBoundary bc(9);
	BOOST_CHECK(bc.apply(index, weights)); 

	for (int i = 0; i < 9; ++i)
		BOOST_CHECK_EQUAL(index[i], result[i]); 
}


BOOST_AUTO_TEST_CASE( test_zero_boundary_needed ) 
{
	std::vector<int> index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11}; 
	std::vector<int> result =     { 0, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0}; 
	std::vector<double> weights = { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13}; 
	std::vector<double> test    = { 0, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0}; 

	CZeroBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
		BOOST_CHECK_EQUAL(weights[i], test[i]); 
	}
}

BOOST_AUTO_TEST_CASE( test_CRepeatBoundary_needed ) 
{
	std::vector<int> index  =     {-1, 0, 1, 2, 3, 4, 5, 6, 7}; 
	std::vector<int> result =     { 0, 0, 1, 2, 3, 4, 5, 5, 5}; 
	std::vector<double> weights = { 1, 2, 3, 4, 5, 6, 7, 6, 8}; 

	CRepeatBoundary bc(6);
	BOOST_CHECK(!bc.apply(index, weights)); 

	for (int i = 0; i < 10; ++i) {
		BOOST_CHECK_EQUAL(index[i], result[i]); 
	}
}

