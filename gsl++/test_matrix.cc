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
#include <gsl++/matrix.hh>

using namespace gsl; 
using namespace ::boost;
using namespace ::boost::unit_test;



BOOST_AUTO_TEST_CASE( test_matrix_alloc_and_free ) 
{
	Matrix m(2,3, true); 
	BOOST_CHECK_EQUAL(m.rows(), 2u); 
	BOOST_CHECK_EQUAL(m.cols(), 3u); 

	for(size_t i = 0; i < 2;++i) 
		for(size_t j = 0; j < 2;++j) 
			BOOST_CHECK_EQUAL(m(i,j), 0.0); 

	m.set(1,2, 1.0); 
	BOOST_CHECK_EQUAL(m(1,2), 1.0); 
	
	Matrix k(m); 
	BOOST_CHECK_EQUAL(k.rows(), 2u); 
	BOOST_CHECK_EQUAL(k.cols(), 3u); 
	BOOST_CHECK_EQUAL(k(1,2), 1.0); 

	k.set(0,2, 2.0); 
	BOOST_CHECK_EQUAL(k(0,2), 2.0); 

	m = k; 
	BOOST_CHECK_EQUAL(k(0,2), 2.0); 

}
