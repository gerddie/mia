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

#include <mia/internal/autotest.hh>
#include <mia/core/simpson.hh>

NS_MIA_USE;
using namespace std;

struct LinearFunction {
	double operator()(double x) const {
		return x;
	}
};

BOOST_AUTO_TEST_CASE( test_integrate_linear2 )
{
	double result = simpson(2, 7, 2, LinearFunction());
	BOOST_CHECK_CLOSE(result, 22.5, 0.01);
}


BOOST_AUTO_TEST_CASE( test_integrate_linear1 )
{
	double result = simpson(2, 7, 1, LinearFunction());
	BOOST_CHECK_CLOSE(result, 22.5, 0.01);
}

BOOST_AUTO_TEST_CASE( test_integrate_linea3 )
{
	double result = simpson(2, 7, 3, LinearFunction());
	BOOST_CHECK_CLOSE(result, 22.5, 0.01);
}
