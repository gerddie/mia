/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#define BOOST_TEST_MODULE VECTOR2DPARAM_TEST
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK
#include <cassert>
#include <iostream>
#include <cmath>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/2d/2DVector.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;


BOOST_AUTO_TEST_CASE( test_2dvector_parameter )
{
	T2DVector<int> intv_value(1, 2);

	TParameter<T2DVector<int> > param(intv_value, true, "a float vector value");

	BOOST_CHECK_EQUAL(intv_value , T2DVector<int>(1,2));

	param.set("<3,12>");
	BOOST_CHECK_EQUAL(intv_value , T2DVector<int>(3,12));


	BOOST_CHECK_THROW(param.set("<3,12.2"), std::invalid_argument);
}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	mia::CCmdOptionList().parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
