/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#define VSTREAM_DOMAIN "test-transio"
#include <mia/internal/autotest.hh>

#include <mia/3d/quaternion.hh>
#include <mia/core/utils.hh>

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

struct QuaternionMatrixEqualFixture {
	void check(const C3DDVector& euler, const C3DFMatrix& m); 
}; 

BOOST_FIXTURE_TEST_CASE(test_get_3x3matrix_rotz, QuaternionMatrixEqualFixture )
{
	C3DDVector rotz(0.0, 0.0, M_PI/3.0); 
	check(rotz, C3DFMatrix(C3DFVector(0.5, -sqrt(3.0)/2.0, 0), 
			       C3DFVector(sqrt(3.0)/2.0, 0.5, 0), 
			       C3DFVector(0.0, 0.0, 1.0)
		      )
		);
}

BOOST_FIXTURE_TEST_CASE(test_get_3x3matrix_roty, QuaternionMatrixEqualFixture )
{

	C3DDVector roty(0.0, M_PI/3.0, 0.0); 
	check(roty, C3DFMatrix(C3DFVector( 0.5,            0,  sqrt(3.0)/2.0), 
			       C3DFVector( 0.0,          1.0,            0.0), 
			       C3DFVector(-sqrt(3.0)/2.0,  0,            0.5)));

}
BOOST_FIXTURE_TEST_CASE(test_get_3x3matrix_rotx, QuaternionMatrixEqualFixture )
{

	C3DDVector rotx(M_PI/3.0, 0.0, 0.0); 
	check(rotx, C3DFMatrix(C3DFVector(1.0, 0.0,  0.0), 
			       C3DFVector(0, 0.5,  -sqrt(3.0)/2.0), 
			       C3DFVector(0, sqrt(3.0)/2.0, 0.5)));
}

BOOST_AUTO_TEST_CASE(test_from_3x3matrix_z )
{
	C3DDVector rotz(0.0, 0.0, M_PI/3.0); 
	
	C3DFMatrix m(C3DFVector(0.5, -sqrt(3.0)/2.0, 0), 
		     C3DFVector(sqrt(3.0)/2.0, 0.5, 0), 
		     C3DFVector(0.0, 0.0, 1.0)
		); 
	
	
	Quaternion a(m); 
	Quaternion b(rotz);

	cvdebug() << a << ", " << b << "\n"; 

	BOOST_CHECK_CLOSE(a.w(), b.w(), 0.1); 
	BOOST_CHECK_CLOSE(a.x(), b.x(), 0.1); 
	BOOST_CHECK_CLOSE(a.y(), b.y(), 0.1); 
	BOOST_CHECK_CLOSE(a.z(), b.z(), 0.1); 


	cvdebug() << "From Matrix:" << a.get_rotation_matrix() << "\n"; 

	cvdebug() << "From Quaternion:" << b.get_rotation_matrix() << "\n"; 
	


}

BOOST_AUTO_TEST_CASE(test_from_3x3matrix_y )
{
	
	C3DDVector roty(0.0, M_PI/3.0, 0.0); 
	C3DFMatrix m(C3DFVector( 0.5,            0,  sqrt(3.0)/2.0), 
		     C3DFVector( 0.0,          1.0,            0.0), 
		     C3DFVector(-sqrt(3.0)/2.0,  0,            0.5));

	
	Quaternion a(m); 
	Quaternion b(roty);

	BOOST_CHECK_CLOSE(a.w(), b.w(), 0.1); 
	BOOST_CHECK_CLOSE(a.x(), b.x(), 0.1); 
	BOOST_CHECK_CLOSE(a.y(), b.y(), 0.1); 
	BOOST_CHECK_CLOSE(a.z(), b.z(), 0.1); 

}

BOOST_AUTO_TEST_CASE(test_from_3x3matrix_x )
{
	
	C3DDVector rotx(M_PI/3.0, 0.0, 0.0); 
	C3DFMatrix m(C3DFVector(1.0, 0.0,  0.0), 
		     C3DFVector(0, 0.5,  -sqrt(3.0)/2.0), 
		     C3DFVector(0, sqrt(3.0)/2.0, 0.5));

	
	Quaternion a(m); 
	Quaternion b(rotx);

	BOOST_CHECK_CLOSE(a.w(), b.w(), 0.1); 
	BOOST_CHECK_CLOSE(a.x(), b.x(), 0.1); 
	BOOST_CHECK_CLOSE(a.y(), b.y(), 0.1); 
	BOOST_CHECK_CLOSE(a.z(), b.z(), 0.1); 

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


BOOST_FIXTURE_TEST_CASE( test_from_rot_matrix_x, QuaternionFixture) 
{
	float rx = M_PI/3.0; 
	float sx, cx; 
	sincosf(rx, &sx, &cx); 
	
	const C3DFMatrix rotx(C3DFVector(  1,  0,   0), 
			      C3DFVector(  0, cx, -sx), 
			      C3DFVector(  0, sx, cx)); 
	
	Quaternion qx(rotx);
	CHECK_Quaternion_CLOSE(qx, Quaternion(sqrt(3.0)/2.0, 0.5, 0, 0),0.1);
}

BOOST_FIXTURE_TEST_CASE( test_from_rot_matrix_y, QuaternionFixture) 
{
	float r = M_PI/3.0; 
	float s, c; 
	sincosf(r, &s, &c);
	
	const C3DFMatrix rot(C3DFVector( c, 0, -s), 
			     C3DFVector( 0, 1,  0), 
			     C3DFVector( s, 0, c)); 
	
	Quaternion q(rot);
	CHECK_Quaternion_CLOSE(q, Quaternion(sqrt(3.0)/2.0, 0, -0.5, 0),0.1);
}

BOOST_FIXTURE_TEST_CASE( test_from_rot_matrix_z, QuaternionFixture) 
{
	float r = M_PI/3.0; 
	float s, c; 
	sincosf(r, &s, &c);
	
	const C3DFMatrix rot(C3DFVector( c,-s, 0), 
			     C3DFVector( s, c, 0), 
			     C3DFVector( 0, 0, 1)); 
	
	Quaternion q(rot);
	CHECK_Quaternion_CLOSE(q, Quaternion(sqrt(3.0)/2.0, 0, 0, 0.5),0.1);
}

BOOST_FIXTURE_TEST_CASE( test_get_rot_matrix_z, QuaternionFixture) 
{
	Quaternion q(sqrt(3.0)/2.0, 0, 0, 0.5); 
	C3DFMatrix rot = q.get_rotation_matrix(); 
	

	float r = M_PI/3.0; 
	float s, c; 
	sincosf(r, &s, &c);
	
	BOOST_CHECK_CLOSE(rot.x.x, c, 0.1); 
	BOOST_CHECK_CLOSE(rot.x.y, -s, 0.1); 
	BOOST_CHECK_CLOSE(rot.y.x,  s, 0.1); 
	BOOST_CHECK_CLOSE(rot.y.y, c, 0.1); 
	BOOST_CHECK_CLOSE(rot.z.z, 1.0, 0.1); 

	BOOST_CHECK_SMALL(rot.x.z,0.0001f); 
	BOOST_CHECK_SMALL(rot.y.z,0.0001f); 
	BOOST_CHECK_SMALL(rot.z.x,0.0001f); 
	BOOST_CHECK_SMALL(rot.z.y,0.0001f); 

}

BOOST_FIXTURE_TEST_CASE( test_get_rot_matrix_y, QuaternionFixture) 
{
	Quaternion q(sqrt(3.0)/2.0, 0, -0.5, 0); 
	C3DFMatrix rot = q.get_rotation_matrix(); 
	

	float r = M_PI/3.0; 
	float s, c; 
	sincosf(r, &s, &c);
	
	BOOST_CHECK_CLOSE(rot.x.x, c, 0.1); 
	BOOST_CHECK_CLOSE(rot.x.z, -s, 0.1); 
	BOOST_CHECK_CLOSE(rot.z.x,  s, 0.1); 
	BOOST_CHECK_CLOSE(rot.z.z, c, 0.1); 
	BOOST_CHECK_CLOSE(rot.y.y, 1.0, 0.1); 

	BOOST_CHECK_SMALL(rot.x.y,0.0001f); 
	BOOST_CHECK_SMALL(rot.y.z,0.0001f); 
	BOOST_CHECK_SMALL(rot.y.x,0.0001f); 
	BOOST_CHECK_SMALL(rot.z.y,0.0001f); 

}

BOOST_FIXTURE_TEST_CASE( test_get_rot_matrix_x, QuaternionFixture) 
{
	Quaternion q(sqrt(3.0)/2.0, 0.5, 0, 0); 
	C3DFMatrix rot = q.get_rotation_matrix(); 
	

	float r = M_PI/3.0; 
	float s, c; 
	sincosf(r, &s, &c);
	
	BOOST_CHECK_CLOSE(rot.x.x, 1, 0.1); 
	BOOST_CHECK_CLOSE(rot.y.z,-s, 0.1); 
	BOOST_CHECK_CLOSE(rot.z.y, s, 0.1); 
	BOOST_CHECK_CLOSE(rot.z.z, c, 0.1); 
	BOOST_CHECK_CLOSE(rot.y.y, c, 0.1); 

	BOOST_CHECK_SMALL(rot.x.y,0.0001f); 
	BOOST_CHECK_SMALL(rot.x.z,0.0001f); 
	BOOST_CHECK_SMALL(rot.y.x,0.0001f); 
	BOOST_CHECK_SMALL(rot.z.x,0.0001f); 

}

BOOST_FIXTURE_TEST_CASE( test_get_rot_matrix, QuaternionFixture) 
{
	Quaternion q(sqrt(3.0)/2.0, 0.5, 1.9, 40.0); 
	q.normalize(); 
	C3DFMatrix rot = q.get_rotation_matrix(); 

	Quaternion q2(rot); 

	BOOST_CHECK_CLOSE(q2.x(), q.x(), 0.1); 
	BOOST_CHECK_CLOSE(q2.y(), q.y(), 0.1); 
	BOOST_CHECK_CLOSE(q2.z(), q.z(), 0.1); 
	BOOST_CHECK_CLOSE(q2.w(), q.w(), 0.1); 

}

QuaternionFixture::QuaternionFixture():
	q1(1, 2, 3, 4), 
	q2(2, 4, 2, 1)
{
}


void QuaternionMatrixEqualFixture::check(const C3DDVector& euler, const C3DFMatrix& t)
{

	Quaternion q(euler); 
	C3DFMatrix m = q.get_rotation_matrix(); 	

	BOOST_CHECK_CLOSE(m.x.x, t.x.x, 0.1); 
	BOOST_CHECK_CLOSE(m.x.y, t.x.y, 0.1); 
	BOOST_CHECK_CLOSE(m.x.z, t.x.z, 0.1); 

	BOOST_CHECK_CLOSE(m.y.x, t.y.x, 0.1); 
	BOOST_CHECK_CLOSE(m.y.y, t.y.y, 0.1); 
	BOOST_CHECK_CLOSE(m.y.z, t.y.z, 0.1); 

	BOOST_CHECK_CLOSE(m.z.x, t.z.x, 0.1); 
	BOOST_CHECK_CLOSE(m.z.y, t.z.y, 0.1); 
	BOOST_CHECK_CLOSE(m.z.z, t.z.z, 0.1); 

}


