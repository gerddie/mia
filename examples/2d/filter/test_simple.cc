/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

// the plug-in tester function tests whether a plug-in is created according to the specification 
#include <mia/internal/plugintester.hh>

/* the filtertest fnction takes input data in a given data typa and an expected result in the expected data 
   type and the filter to be tested. 
   A test attribute is also created and it is tested whether the filter properly passed this attribute. 
*/ 
#include <mia/template/filtertest.hh>

// this gives the declaration of the filter to be tested
#include <mia/2d/filter/simple.hh>


NS_MIA_USE
using namespace std;
using namespace simple_2dimage_filter;

/*
  The first test checks whether the filter actually throws if the input data is of an unsupported pixel type.
*/


BOOST_FIXTURE_TEST_CASE(test_simple_1, TFiltertestFixture<T2DImage>)
{
	const int src[6] =   {  0,   1,  2,  3,  4,  5};
	const int test[6] =  {  0,  -1, -2, -3, -4, -5};
	C2DBounds size(3,2);

	C2DSIImage input(size, src); 

	auto filter = BOOST_TEST_create_from_plugin<C2DSimpleImageFilterPlugin>("simple:a=-1"); 
	
	BOOST_CHECK_THROW(run(size, src, size, test, *filter), invalid_argument); 
}


/*
  This test runs a valid filter.  
*/

BOOST_FIXTURE_TEST_CASE(test_simple_3, TFiltertestFixture<T2DImage>)
{
	const float src[6] =   {  0,  1,  2,  3,  4,  5};
	const float test[6] =  {  0,  2,  4,  6,  8, 10};
	C2DBounds size(3,2);

	auto filter = BOOST_TEST_create_from_plugin<C2DSimpleImageFilterPlugin>("simple:a=2"); 
	
	run(size, src, size, test, *filter); 
}

/*
  This test runs another valid filter with different filter parameters.  
*/

BOOST_FIXTURE_TEST_CASE(test_simple_2, TFiltertestFixture<T2DImage>)
{
	const double src[6] =   {  0,   1,  2,  3,  4,  5};
	const double test[6] =  {  0,  -1, -2, -3, -4, -5};
	C2DBounds size(3,2);

	auto filter = BOOST_TEST_create_from_plugin<C2DSimpleImageFilterPlugin>("simple:a=-1"); 
	
	run(size, src, size, test, *filter); 
}

