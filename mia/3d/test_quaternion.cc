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

#define VSTREAM_DOMAIN "test-transio"
#include <mia/internal/autotest.hh>

#include <mia/3d/quaternion.hh>

NS_MIA_USE; 

struct QuaternionFixture {
	QuaternionFixture(); 
	Quaternion q1; 
	Quaternion q2; 
}; 

BOOST_FIXTURE_TEST_CASE( test_quaternion_basics, QuaternionFixture) 
{
	Quaternion q1(1, 2, 3, 4); 
	Quaternion q2(2, 4, 2, 1); 

	BOOST_CHECK(q1 == q1); 
	BOOST_CHECK(q1 != q2); 
	BOOST_CHECK(q2 != q1); 

	Quaternion q3(q1); 
	
	BOOST_CHECK_EQUAL(q1,q3);
	BOOST_CHECK(q2 != q3); 

	
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_norm, QuaternionFixture) 
{
	BOOST_CHECK_CLOSE(q1.norm(), sqrt(1.0+4.0+9.0+16.0),0.1); 

	q1.normalize(); 
	BOOST_CHECK_CLOSE(q1.norm(),1.0, 0.1); 
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_add, QuaternionFixture) 
{
	Quaternion sum(3,6,5,5);
	q1 += q2; 
	BOOST_CHECK_EQUAL(q1, sum); 
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_sub, QuaternionFixture) 
{
	Quaternion diff(-1,-2,1,3);
	q1 -= q2; 
	BOOST_CHECK_EQUAL(q1, diff); 
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_invese_rot, QuaternionFixture) 
{
	Quaternion inv(1, -2, -3, -4);
	BOOST_CHECK_EQUAL(q1.inverse(), inv); 
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_prod, QuaternionFixture) 
{
	Quaternion prod(-16,3,22,1);
	q1 *= q2; 
	BOOST_CHECK_EQUAL(q1, prod); 
}

static void CHECK_Quaternion_CLOSE(const Quaternion& a, const Quaternion& b, double tol)
{
	BOOST_CHECK_CLOSE(a.w(), b.w(), tol); 
	BOOST_CHECK_CLOSE(a.x(), b.x(), tol); 
	BOOST_CHECK_CLOSE(a.y(), b.y(), tol); 
	BOOST_CHECK_CLOSE(a.z(), b.z(), tol); 
}

BOOST_FIXTURE_TEST_CASE( test_from_euler, QuaternionFixture) 
{
	C3DDVector rot(0.0, 0.0, 0.0); 
	Quaternion q(rot);

	BOOST_CHECK_EQUAL(q, Quaternion(1.0, 0, 0, 0)); 

	C3DDVector rotz(0.0, 0.0, M_PI/3.0); 
	Quaternion qz(rotz);

	CHECK_Quaternion_CLOSE(qz, Quaternion(sqrt(3.0)/2.0, 0, 0, 0.5),0.1); 
	
	C3DDVector rotx(M_PI/3.0, 0.0, 0.0); 
	Quaternion qx(rotx);
	
	CHECK_Quaternion_CLOSE(qx, Quaternion(sqrt(3.0)/2.0, 0.5, 0, 0),0.1); 
	
	C3DDVector roty(0.0, M_PI/3.0, 0.0); 
	Quaternion qy(roty);
	CHECK_Quaternion_CLOSE(qy, Quaternion(sqrt(3.0)/2.0, 0, 0.5, 0),0.1); 
	
}

#if 0
BOOST_FIXTURE_TEST_CASE( test_euler, QuaternionFixture) 
{
	C3DDVector rot(0.5, 1.4, 1.1); 
	Quaternion q(rot);
	C3DDVector qrot = q.get_euler_angles(); 
	BOOST_CHECK_CLOSE(qrot.x, rot.x, 0.1); 
	BOOST_CHECK_CLOSE(qrot.y, rot.y, 0.1); 
	BOOST_CHECK_CLOSE(qrot.z, rot.z, 0.1); 
	Quaternion q2(qrot);
	C3DDVector q2rot = q2.get_euler_angles(); 

	BOOST_CHECK_CLOSE(q2rot.x, qrot.x, 0.1); 
	BOOST_CHECK_CLOSE(q2rot.y, qrot.y, 0.1); 
	BOOST_CHECK_CLOSE(q2rot.z, qrot.z, 0.1); 

}

#endif


QuaternionFixture::QuaternionFixture():
	q1(1, 2, 3, 4), 
	q2(2, 4, 2, 1)
{
}



