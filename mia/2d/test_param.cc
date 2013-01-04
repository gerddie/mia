/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define BOOST_TEST_MODULE VECTOR2DPARAM_TEST
#include <cassert>
#include <iostream>
#include <cmath>
#include <climits>

#include <mia/internal/autotest.hh>

#include <mia/2d/vector.hh>
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
