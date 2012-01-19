/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <sstream>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core.hh>
#include <mia/3d/3DVector.hh>

using namespace std;
NS_MIA_USE

static void test_3dvectors()
{

	C3DFVector a(1.0f, 2.0f, 3.0f);
	C3DFVector b(1.4f, 2.3f, 4.0f);
	
	C3DFVector cprod(1.4f, 4.6f, 12.0f); 
	BOOST_CHECK_EQUAL(a*b, cprod); 
	
	C3DFVector c(2.0f, 3.0f, 4.0f);
	C3DFVector d(4.4f, 9.0f,20.0f);
	C3DFVector d_by_c(2.2f, 3.0f, 5.0f); 

	BOOST_CHECK_EQUAL(d/c, d_by_c); 

	
	

	

	ostringstream so;
	so << a;
	cvdebug() << so.str() << "\n";
	BOOST_CHECK(so.str() == string("<1,2,3>"));

	C3DFVector sc(3.0f, 4.0f, 1.0f);
	istringstream si("<1.0,2.0,3.0>");
	si >> sc;
	BOOST_CHECK (sc == a);

	BOOST_CHECK(a.size() == 3);
	BOOST_CHECK(a.norm2() == 14.0f);


	c = a + b;
	cvdebug() << c << "\n";
	BOOST_CHECK(c.x == 2.4f && c.y == 4.3f && c.z == 7.0f);

	c = b + a;
	BOOST_CHECK(c.x == 2.4f && c.y == 4.3f && c.z == 7.0f);

	c = b - a;
	BOOST_CHECK(fabs(c.x - 0.4f) < 0.0001 &&
	       fabs(c.y - 0.3f) < 0.0001 &&
	       fabs(c.z - 1.0f) < 0.0001 );

	c = 2.0f * a;
	BOOST_CHECK(fabs(c.x - 2.0f) < 0.0001 &&
	       fabs(c.y - 4.0f) < 0.0001 &&
	       fabs(c.z - 6.0f) < 0.0001 );

	c = a / 2.0f;
	BOOST_CHECK(fabs(c.x - 0.5f) < 0.0001 &&
	       fabs(c.y - 1.0f) < 0.0001 &&
	       fabs(c.z - 1.5f) < 0.0001 );

	float dot_prod =  dot(a, b);
	BOOST_CHECK(fabs(dot_prod - 18.0f) < 0.0001);

	BOOST_CHECK(fabs(a.norm() - sqrt(14.0f)) < 0.0001);

	BOOST_CHECK(a != b);
	c = b;
	BOOST_CHECK(c == b);

	

}

static void test_float_vector_option()
{
	C3DFVector v(1,2,3);

	PCmdOption popt(make_opt(v,  "3dvector", 'f', "a float 3d vector option", "3dvector"));
	const char *str_value = "<1.2,3.4,8.2>";
	try {
		popt->set_value(str_value);
		BOOST_CHECK(v.x == 1.2f && v.y == 3.4f && v.z == 8.2f);
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	const char *str_value_err = "1.2,3.4,8.2";
	try {
		popt->set_value(str_value_err);
		BOOST_FAIL("error value not detected");
	}
	catch (invalid_argument& x) {
		BOOST_MESSAGE(string("Caught:") + string(x.what()));
	}
}

void test_size_vector_option()
{
	C3DBounds v(1,2,3);

	PCmdOption popt(make_opt(v,  "3dbounds", 'f', "a 3d size option", "3dbounds"));
	const char *str_value = "<12,34,256>";
	try {
		cvdebug() << "initialising from '" << str_value<< "'\n";
		popt->set_value(str_value);
		BOOST_CHECK(v.x == 12 && v.y == 34 && v.z == 256);
	}
	catch (invalid_argument& x) {
		BOOST_FAIL(x.what());
	}

	const char *str_value_err = "<1.2,3.4,8.2>";
	try {
		popt->set_value(str_value_err);
		BOOST_FAIL("error value not detected");
	}
	catch (invalid_argument& x) {
		BOOST_MESSAGE(string("Caught:") + string(x.what()));
	}
}

static void test_swizzle()
{
	T3DVector<int> test(1,2,3);

	BOOST_CHECK_EQUAL(test.xyz(), test);
	BOOST_CHECK_EQUAL(test.xzy(), T3DVector<int>(1,3,2));
	BOOST_CHECK_EQUAL(test.yxz(), T3DVector<int>(2,1,3));
	BOOST_CHECK_EQUAL(test.yzx(), T3DVector<int>(2,3,1));
	BOOST_CHECK_EQUAL(test.zxy(), T3DVector<int>(3,1,2));
	BOOST_CHECK_EQUAL(test.zyx(), T3DVector<int>(3,2,1));


}

static void test_fill()
{
	T3DVector<int> test;

	BOOST_CHECK_EQUAL(test.x, 0);
	BOOST_CHECK_EQUAL(test.y, 0);
	BOOST_CHECK_EQUAL(test.z, 0);

	test.fill(2); 
	BOOST_CHECK_EQUAL(test.z, 2);
	BOOST_CHECK_EQUAL(test.y, 2);
	BOOST_CHECK_EQUAL(test.x, 2);


}


void add_3dvector_tests(boost::unit_test::test_suite* suite)
{
	suite->add( BOOST_TEST_CASE( &test_3dvectors));
	suite->add( BOOST_TEST_CASE( &test_float_vector_option));
	suite->add( BOOST_TEST_CASE( &test_size_vector_option));
	suite->add( BOOST_TEST_CASE( &test_swizzle ));
	suite->add( BOOST_TEST_CASE( &test_fill ));
}
