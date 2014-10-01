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

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>

#include <gsl++/vector.hh>

#include <vector>
#include <iostream>


using namespace gsl; 
using namespace ::boost;
using namespace ::boost::unit_test;

namespace bmpl=boost::mpl;

typedef bmpl::vector<DoubleVector, 
		     FloatVector, 
		     LongVector, 
		     IntVector, 
		     ShortVector, 
		     CharVector,
		     ULongVector,
		     UIntVector,
		     UShortVector,
		     UCharVector>  test_types;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_vector_zero_init, T ,test_types ) 
{									
	T gsl_vector(5,true);

	BOOST_REQUIRE(gsl_vector.size() == 5u); 

	for(size_t i = 0; i < 5; ++i) {
		BOOST_CHECK_EQUAL(gsl_vector[i], 0.0);
	}

	
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_vector_copy_to, T ,test_types ) 
{									
	
	const typename T::value_type init[5] = {1, 2, 3, 4, 5}; 
	std::vector<typename T::value_type> a(init, init+5); 
	T gsl_vector(5,false);

	BOOST_REQUIRE(gsl_vector.size()== 5); 
	copy(a.begin(), a.end(), gsl_vector.begin()); 
	
	for(size_t i = 0; i < 5; ++i) {
		BOOST_CHECK_EQUAL(gsl_vector[i], static_cast<typename T::value_type>(i + 1));
	}

	
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_vector_copy_from, T ,test_types ) 
{									
	T gsl_vector(5,false);

	BOOST_REQUIRE(gsl_vector.size()== 5); 

	for(size_t i = 0; i < 5; ++i)
		gsl_vector[i] = i+1; 
	
	std::vector<typename T::value_type> a(5,6); 

	BOOST_REQUIRE(gsl_vector.begin() != gsl_vector.end()); 
	BOOST_REQUIRE(!(gsl_vector.begin() == gsl_vector.end())); 
	copy(gsl_vector.begin(), gsl_vector.end(), a.begin()); 
	
	for(size_t i = 0; i < 5; ++i)
		BOOST_CHECK_EQUAL(a[i], static_cast<typename T::value_type>(i + 1)); 

	
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_vector_inplace_add, T ,test_types ) 
{									
	T gsl_vector(5,true);

	BOOST_REQUIRE(gsl_vector.size()== 5); 

	for(size_t i = 0; i < 5; ++i) {
		gsl_vector[i] = i; 
		gsl_vector[i] += 1; 
	}
	

	for(size_t i = 0; i < 5; ++i)
		BOOST_CHECK_EQUAL(gsl_vector[i], static_cast<typename T::value_type>(i + 1)); 

	
}



BOOST_AUTO_TEST_CASE( test_vector_non_owning ) 
{									
	gsl_vector *x = gsl_vector_calloc(10); 
	DoubleVector wx(x); 
	
	const size_t i2 = 2; 

	wx[i2] = 2.0; 
	
	BOOST_CHECK_EQUAL(gsl_vector_get(x,2), 2.0); 
	
	gsl_vector_free(x); 
}


BOOST_AUTO_TEST_CASE( test_vector_copy ) 
{
	DoubleVector wx(10, false); 
	DoubleVector wy(5, false); 
	
	const size_t i2 = 2; 
	wx[i2] = 3.0; 
	BOOST_CHECK_EQUAL(wx[i2], 3.0); 

	wy[i2] = 2.0; 
	BOOST_CHECK_EQUAL(wy[i2], 2.0); 
	
	BOOST_CHECK_EQUAL(wx.size(), 10u); 
	wx = wy; 
	BOOST_CHECK_EQUAL(wx.size(), 5u); 
	BOOST_CHECK_EQUAL(wx[i2], 2.0); 
}

BOOST_AUTO_TEST_CASE( test_vector_iterator ) 
{
	DoubleVector v(50, false); 
	for (int i = 0; i < 50; ++i)
		v[i] = i + 0; 

	auto it_dv = v.begin(); 
	for (int i = 0; i < 50; ++i, ++it_dv)
		BOOST_CHECK_EQUAL(*it_dv, i); 
	

	gsl_vector_view vview = gsl_vector_subvector_with_stride (v, 2, 2, 20); 
	
	DoubleVector dv_view(&vview.vector); 
	
	for (int i = 0; i < 20; ++i) {
		BOOST_CHECK_EQUAL(dv_view[i], 2*i + 2); 
	}

	auto it_dv_base = dv_view.begin(); 
	auto it_dv_view = dv_view.begin(); 

	BOOST_CHECK(it_dv_base ==  it_dv_view); 
	BOOST_CHECK(it_dv_base <=  it_dv_view); 
	BOOST_CHECK(it_dv_base >=  it_dv_view); 
	
	BOOST_CHECK_EQUAL(*it_dv_view, 2); 
	it_dv_view += 3; 

	BOOST_CHECK(it_dv_base <  it_dv_view); 
	BOOST_CHECK(it_dv_base <=  it_dv_view); 
	BOOST_CHECK(it_dv_view >= it_dv_base); 
	BOOST_CHECK(it_dv_view != it_dv_base); 


	BOOST_CHECK_EQUAL(*it_dv_view, 8); 

	it_dv_view -= -2; 
	BOOST_CHECK_EQUAL(*it_dv_view, 12);

	
		
}


static void test_const_vector_iterator_impl(const DoubleVector& v) 
{
	auto it_dv = v.begin(); 
	for (int i = 0; i < 50; ++i, ++it_dv)
		BOOST_CHECK_EQUAL(*it_dv, i); 
	
	
	gsl_vector_const_view vview = gsl_vector_const_subvector_with_stride (v, 2, 2, 20); 
	
	const DoubleVector dv_view(&vview.vector); 
	
	for (int i = 0; i < 20; ++i) {
		BOOST_CHECK_EQUAL(dv_view[i], 2*i + 2); 
	}
	
	auto it_dv_base = dv_view.begin(); 
	auto it_dv_view = dv_view.begin(); 

	BOOST_CHECK(it_dv_base ==  it_dv_view); 
	BOOST_CHECK(it_dv_base <=  it_dv_view); 
	BOOST_CHECK(it_dv_base >=  it_dv_view); 
	
	BOOST_CHECK_EQUAL(*it_dv_view, 2); 
	it_dv_view += 3; 

	BOOST_CHECK(it_dv_base <  it_dv_view); 
	BOOST_CHECK(it_dv_base <=  it_dv_view); 
	BOOST_CHECK(it_dv_view >= it_dv_base); 
	BOOST_CHECK(it_dv_view != it_dv_base); 
	

	BOOST_CHECK_EQUAL(*it_dv_view, 8); 
	
	it_dv_view -= -2; 
	BOOST_CHECK_EQUAL(*it_dv_view, 12);
	
}

BOOST_AUTO_TEST_CASE( test_const_vector_iterator ) 
{
	DoubleVector v(50, false); 
	for (int i = 0; i < 50; ++i)
		v[i] = i + 0; 

	test_const_vector_iterator_impl(v); 
}


