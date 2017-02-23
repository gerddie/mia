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

#include <mia/internal/autotest.hh>
#include <mia/3d/matrix.hh>

NS_MIA_USE; 

struct FixtureMatrixTest {
	void run(const C3DFMatrix& m, const C3DFVector& test_values, 
		 const C3DFVector& ev1, const C3DFVector& ev2, const C3DFVector& ev3, int type); 
}; 

BOOST_AUTO_TEST_CASE( test_matrix_rank_3 ) 
{
	BOOST_CHECK_EQUAL(C3DFMatrix::_1.get_rank(), 3);
}

BOOST_AUTO_TEST_CASE( test_matrix_rank_2 ) 
{
	BOOST_CHECK_EQUAL(C3DFMatrix::diagonal(C3DFVector(1,1,0)).get_rank(), 2);
}

BOOST_AUTO_TEST_CASE( test_matrix_rank_1 ) 
{
	BOOST_CHECK_EQUAL(C3DFMatrix::diagonal(C3DFVector(1,0,0)).get_rank(), 1);
}

BOOST_AUTO_TEST_CASE( test_matrix_rank_0 ) 
{
	BOOST_CHECK_EQUAL(C3DFMatrix::diagonal(0).get_rank(), 0);
}



BOOST_AUTO_TEST_CASE( test_matrix_determinant ) 
{
	BOOST_CHECK_EQUAL(C3DFMatrix::_1.get_det(), 1.0); 

	C3DFMatrix m(C3DFVector(1,2,1), C3DFVector(4,5,6), C3DFVector(7,8,9)); 
	
	BOOST_CHECK_EQUAL(m.get_det(), 6.0); 
}


BOOST_FIXTURE_TEST_CASE( test_matrix_eigenstuff_all_real, FixtureMatrixTest ) 
{
	C3DFVector test_values(3, 2, 1); 
	C3DFVector ev1(cos(0.3),  -sin(0.3),   0.0);   
	C3DFVector ev2(sin(0.3),  cos(0.3),   0.0);   
	C3DFVector ev3(     0.0,       0.0,   1.0);   
	C3DFMatrix m(ev1, ev2, ev3); 

	cvdebug() << m * C3DFMatrix::diagonal(test_values) * m.transposed() << "\n"; 

	run (m.transposed() * C3DFMatrix::diagonal(test_values) * m, test_values, ev1, ev2, ev3, 3); 
}

/* disable and rethink interface 
// this test failes, because only one eigenvector for thhe equal eigenvalues is 
// evaluated while there should actually by two and they only have to be orthogonal 
// to each other and to the third one, but they can still rotate
// around the axis of the third EVec 

BOOST_FIXTURE_TEST_CASE( test_matrix_eigenstuff_all_real_two_equal, FixtureMatrixTest ) 
{

	C3DFVector test_values(3, 2, 2); 
	C3DFVector ev1(cos(0.3),  -sin(0.3),   0.0);   
	C3DFVector ev2(sin(0.3),  cos(0.3),   0.0);   
	C3DFVector ev3(     0.0,       0.0,   1.0);   
	C3DFMatrix m(ev1, ev2, ev3); 

	cvdebug() << m * C3DFMatrix::diagonal(test_values) * m.transposed() << "\n"; 

	run (m.transposed() * C3DFMatrix::diagonal(test_values) * m, test_values, ev1, ev2, ev3, 2); 
}

*/

void FixtureMatrixTest::run(const C3DFMatrix& m, const C3DFVector& test_values, 
			    const C3DFVector& ev1, const C3DFVector& ev2, const C3DFVector& ev3, int type)
{
	
	C3DFVector eval; 
	BOOST_CHECK_EQUAL(m.get_eigenvalues(eval), type); 
	
	BOOST_CHECK_CLOSE(eval.x, test_values.x, 0.1); 
	BOOST_CHECK_CLOSE(eval.y, test_values.y, 0.1); 
	BOOST_CHECK_CLOSE(eval.z, test_values.z, 0.1); 
	
	C3DFVector vec =  m.get_real_eigenvector(0); 
	
	BOOST_CHECK_CLOSE(vec.x, ev1.x, 0.1); 
	BOOST_CHECK_CLOSE(vec.y, ev1.y, 0.1); 
	BOOST_CHECK_CLOSE(vec.z, ev1.z, 0.1); 

	vec =  m.get_real_eigenvector(1); 
	
	BOOST_CHECK_CLOSE(vec.x, ev2.x, 0.1); 
	BOOST_CHECK_CLOSE(vec.y, ev2.y, 0.1); 
	BOOST_CHECK_CLOSE(vec.z, ev2.z, 0.1); 

	vec =  m.get_real_eigenvector(2); 
	
	BOOST_CHECK_CLOSE(vec.x, ev3.x, 0.1); 
	BOOST_CHECK_CLOSE(vec.y, ev3.y, 0.1); 
	BOOST_CHECK_CLOSE(vec.z, ev3.z, 0.1); 

	
}

