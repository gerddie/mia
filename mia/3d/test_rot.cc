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
}; 


BOOST_FIXTURE_TEST_CASE( test_identity_from_matrix_string, RotIdentityTestFixture) 
{
	unique_ptr<C3DRotation>  identity(C3DRotation::from_string("rot-matrix=1,0,0,0,1,0,0,0,1"));

	check_matrix(identity->as_matrix_3x3()); 
	
	check_quaternion(identity->as_quaternion()); 
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

