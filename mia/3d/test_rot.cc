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

#include <mia/internal/autotest.hh>
#include <mia/3d/rot.hh>

NS_MIA_USE

using std::unique_ptr; 

struct RotIdentityTestFixture {

	void check_matrix(const C3DDMatrix& matrix) const; 
	void check_quaternion(const Quaternion& q) const; 

	void check_matrix(const C3DDMatrix& matrix, const C3DDMatrix& expect) const; 
	void check_quaternion(const Quaternion& q, const Quaternion& expect) const; 

	void check_small_or_close(double x, double e) const; 
}; 


BOOST_FIXTURE_TEST_CASE( test_identity_from_matrix_string, RotIdentityTestFixture) 
{
	C3DRotation  identity("rot-matrix=1,0,0;0,1,0;0,0,1");

	check_matrix(identity.as_matrix_3x3()); 
	
	check_quaternion(identity.as_quaternion()); 
}

BOOST_FIXTURE_TEST_CASE( test_copying_around, RotIdentityTestFixture ) 
{
	Quaternion q1(0.5,0.5,0.5,0.5); 
	Quaternion q2(-0.5,0.5,-0.5,0.5); 
	
	C3DRotation  r1(q1);
	C3DRotation  r2(q2);

	C3DRotation r1c(r1); 

	check_quaternion(r1c.as_quaternion(), q1); 
	
	r1c = r2;
	
	check_quaternion(r1c.as_quaternion(), q2); 
	
	r1 = r1c; 
	check_quaternion(r1.as_quaternion(), q2);

	r2 = r1c; 

	

}

BOOST_FIXTURE_TEST_CASE( test_identity_from_quaternion_string, RotIdentityTestFixture) 
{
	C3DRotation  identity("rot-quaternion=1,0,0,0");

	check_matrix(identity.as_matrix_3x3()); 
	
	check_quaternion(identity.as_quaternion()); 
}

BOOST_FIXTURE_TEST_CASE( test_base_from_string, RotIdentityTestFixture) 
{
	C3DRotation  identity("rot-identity");
	check_matrix(identity.as_matrix_3x3()); 
	check_quaternion(identity.as_quaternion()); 
}

BOOST_FIXTURE_TEST_CASE( test_matrix_to_from_string, RotIdentityTestFixture) 
{
	Quaternion q(0.5,0.5,0.5,0.5); 
	C3DRotation mr(q.get_rotation_matrix()); 
	
	C3DRotation  rmr(mr.as_string());

	check_matrix(rmr.as_matrix_3x3(), mr.as_matrix_3x3()); 
	
	check_quaternion(rmr.as_quaternion(), q); 
}

BOOST_FIXTURE_TEST_CASE( test_quaternion_to_from_string, RotIdentityTestFixture) 
{
	Quaternion q(0.5,0.5,0.5,0.5); 
	C3DRotation qr(q); 
	
	C3DRotation  rqr(qr.as_string());

	check_matrix(rqr.as_matrix_3x3(), qr.as_matrix_3x3()); 
	
	check_quaternion(rqr.as_quaternion(), q); 
}


void RotIdentityTestFixture::check_matrix(const C3DDMatrix& matrix) const
{
	BOOST_CHECK_EQUAL(matrix.x.x, 1.0); 
	BOOST_CHECK_EQUAL(matrix.y.y, 1.0); 
	BOOST_CHECK_EQUAL(matrix.z.z, 1.0); 

	BOOST_CHECK_SMALL(matrix.x.y, 1e-8); 
	BOOST_CHECK_SMALL(matrix.x.z, 1e-8); 
	BOOST_CHECK_SMALL(matrix.y.x, 1e-8); 
	BOOST_CHECK_SMALL(matrix.y.z, 1e-8); 
	BOOST_CHECK_SMALL(matrix.z.x, 1e-8); 
	BOOST_CHECK_SMALL(matrix.z.y, 1e-8); 
}

void RotIdentityTestFixture::check_quaternion(const Quaternion& q) const
{
	BOOST_CHECK_CLOSE(q.w(), 1.0, 1e-8); 
	BOOST_CHECK_SMALL(q.x(), 1e-5); 
	BOOST_CHECK_SMALL(q.y(), 1e-5); 
	BOOST_CHECK_SMALL(q.z(), 1e-5); 
	
}

void RotIdentityTestFixture::check_small_or_close(double x, double e) const
{
	if (e == 0.0) 
		BOOST_CHECK_SMALL(x, 1e-7); 
	else 
		BOOST_CHECK_CLOSE(x, e, 1e-5); 
}

void RotIdentityTestFixture::check_matrix(const C3DDMatrix& matrix, const C3DDMatrix& expect) const
{
	check_small_or_close(matrix.x.x, expect.x.x); 
	check_small_or_close(matrix.x.y, expect.x.y); 
	check_small_or_close(matrix.x.z, expect.x.z); 

	check_small_or_close(matrix.z.z, expect.z.z); 
	check_small_or_close(matrix.y.y, expect.y.y); 
	check_small_or_close(matrix.y.z, expect.y.z); 


	check_small_or_close(matrix.z.x, expect.z.x); 
	check_small_or_close(matrix.z.y, expect.z.y); 
	check_small_or_close(matrix.z.z, expect.z.z); 
}

void RotIdentityTestFixture::check_quaternion(const Quaternion& q, const Quaternion& expect) const
{
	check_small_or_close(q.x(), expect.x()); 
	check_small_or_close(q.y(), expect.y()); 
	check_small_or_close(q.z(), expect.z()); 
	check_small_or_close(q.w(), expect.w());
}

