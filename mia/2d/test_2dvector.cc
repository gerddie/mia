/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <iostream>

#include <mia/core.hh>
#include <mia/2d/2DVector.hh>

using namespace std;
NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_2dvector )
{
	C2DFVector a(1.0f, 2.0f);
	C2DFVector b(1.4f, 2.3f);

	BOOST_CHECK(a.size() == 2);
	BOOST_CHECK(a.norm2() == 5.0f);

	ostringstream so;
	so << a;
	cvdebug() << so.str() << "\n";
	BOOST_CHECK(so.str() == string("<1,2>"));

	C2DFVector c(3.0f, 4.0f);
	istringstream si("<1.0,2.0>");
	si >> c;

	BOOST_CHECK (c == a);

	c = a + b;
	BOOST_CHECK(c.x == 2.4f && c.y == 4.3f);

	c = b + a;
	BOOST_CHECK(c.x == 2.4f && c.y == 4.3f);

	c = b - a;
	BOOST_CHECK_CLOSE(c.x, 0.4f, 0.0001);
	BOOST_CHECK_CLOSE(c.y, 0.3f, 0.0001);

	c = 2.0f * a;
	BOOST_CHECK_CLOSE(c.x, 2.0f, 0.0001);
	BOOST_CHECK_CLOSE(c.y, 4.0f, 0.0001);

	c = a / 2.0f;
	BOOST_CHECK_CLOSE(c.x, 0.5f, 0.0001);
	BOOST_CHECK_CLOSE(c.y, 1.0f, 0.0001);

	float dot_prod =  dot(a, b);
	BOOST_CHECK_CLOSE(dot_prod, 6.0f, 0.0001);

	BOOST_CHECK_CLOSE(a.norm(),sqrt(5.0f), 0.0001);

	BOOST_CHECK(a != b);

	c = b;
	BOOST_CHECK(c == b);
}


BOOST_AUTO_TEST_CASE(test_float_vector_option)
{
	C2DFVector v(1,2);

	PCmdOption popt(make_opt(v,  "2dvector", 'f', "a float 2d vector option", "2vector"));
	const char *str_value = "<1.2,3.4>";

	popt->set_value(str_value);
	BOOST_CHECK(v.x == 1.2f && v.y == 3.4f);

	const char *str_value_err = "1.2,3.4,8.2";
	BOOST_CHECK_THROW(popt->set_value(str_value_err), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_size_vector_option)
{
	C2DBounds v(1,2);

	PCmdOption popt(make_opt(v,  "2dbounds", 'f', "a 2d size option", "2dbounds"));
	const char *str_value = "<12,34>";

	popt->set_value(str_value);
	BOOST_CHECK(v.x == 12 && v.y == 34 );

	const char *str_value_err = "<1.2,3.4>";
	BOOST_CHECK_THROW( popt->set_value(str_value_err), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_vector_fill)
{
	C2DBounds v; 
	BOOST_CHECK_EQUAL(v.x, 0u); 
	BOOST_CHECK_EQUAL(v.y, 0u); 

	v.fill(2); 
	BOOST_CHECK_EQUAL(v.x, 2u); 
	BOOST_CHECK_EQUAL(v.y, 2u); 

}

