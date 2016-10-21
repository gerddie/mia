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

#ifndef mia_test_testhelpers_hh
#define mia_test_testhelpers_hh

#include <type_traits>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace miatest {

namespace impl {

template <typename T, bool is_integral> 
struct test_equal_or_close {
	static void apply(T value, T expect, double, double) {
		BOOST_CHECK_EQUAL(value, expect); 
	}
}; 

template <typename T> 
struct test_equal_or_close<T, false> {
	static void apply(T value, T expect, double reltol, double abstol) {
		// handle the check against zero case: 
		if (fabs(expect) > 1e-8) 
			BOOST_CHECK_CLOSE(value, expect, reltol); 
		else {
			BOOST_CHECK(fabs(value - expect) < abstol); 
		}
	}
};

}

/**
   \ingroup tests 

   \brief compare helper function for uni tests 
    
   This function is used to hide the uglyness of selecting the proper test method when 
   comparing either integers or floating point values during tests. The function calls 
   BOOST_CHECK_EQUAL for integral types, BOOST_CHECK_CLOSE, if the expected value is larger then 
   1e-8, and BOOST_CHECK if the expected value is smaller then 1e-8 
   \tparam T the type of the values to be compared 
   \param value the value to test 
   \param expect the expected value 
   \param reltol relative tolerance to use if the type T is floating point and \a expect is larger then 1e-8
   \param abstol  absolute tolerance to use if the type T is floating point and \a expect is smaller then 1e-8
*/

template <typename T, typename E> 
void equal_or_close(T value, E expect, double reltol = 0.1, double abstol=1e-12) 
{
	impl::test_equal_or_close<E, std::is_integral<E>::value >::apply(value, expect, reltol, abstol);
}

}

#endif

