/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2011
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

#include <mia/internal/autotest.hh>
#include <mia/3d/cost/ngf.hh>


using namespace std;
using namespace ngf_3dimage_cost; 
using namespace mia; 

struct C3DFVectorfieldFixture {
	
	C3DFVectorfieldFixture(); 
	
	C3DBounds size;
	C3DFVectorfield field; 
}; 


BOOST_FIXTURE_TEST_CASE( test_ngf_evaluator_dot, C3DFVectorfieldFixture )
{
	
	C3DFVectorfield input(C3DBounds(3,3,3)); 
	
	FScalar scalar; 

	BOOST_CHECK_CLOSE( scalar.cost(field(0,0,0), field(1,1,1)), - 26.0f*13.0f, 0.01f); 
	
	double cost = 0.0; 
	auto grad = scalar.grad(3, 9, field.begin() + 9 + 3 + 1, field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, - 26.0f* 13.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.x, 26.0f * 19.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.y, -26.0f * 15.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.z, -26.0f * 31.0f, 0.01f); 
	
}

BOOST_FIXTURE_TEST_CASE( test_ngf_evaluator_cross, C3DFVectorfieldFixture )
{
	
	C3DFVectorfield input(C3DBounds(3,3,3)); 
	
	FCross costfunct; 

	BOOST_CHECK_CLOSE( costfunct.cost(field(0,0,0), field(1,1,1)), 5.0f, 0.01f); 
	
	double cost = 0.0; 
	auto grad = costfunct.grad(3, 9, field.begin() + 9 + 3 + 1, field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, 5.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.x, 18.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.y, 2.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.z, -22.0f, 0.01f); 
	
}

BOOST_FIXTURE_TEST_CASE( test_ngf_evaluator_delta_scalar, C3DFVectorfieldFixture )
{
	
	C3DFVectorfield input(C3DBounds(3,3,3)); 
	
	FDeltaScalar costfunct; 

	BOOST_CHECK_CLOSE( costfunct.cost(field(1,1,1),field(0,0,0)), 5.3330572096f, 0.01f); 
	
	double cost = 0.0; 
	auto grad = costfunct.grad(3, 9, field.begin() + 9 + 3 + 1, field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, 5.3330572096f, 0.01f); 

	BOOST_CHECK_CLOSE( grad.x, -7.7363f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.y, 6.4125f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.z, 12.761, 0.01f); 
	
}




C3DFVectorfieldFixture::C3DFVectorfieldFixture():
	size(3,3,3), 
	field(size)
{
	field(0,0,0) = C3DFVector(1,2,3); 
	field(1,0,0) = C3DFVector(2,3,5); 
	field(2,0,0) = C3DFVector(4,5,1); 

	field(0,1,0) = C3DFVector(2,5,5); 
	field(1,1,0) = C3DFVector(3,2,4); 
	field(2,1,0) = C3DFVector(5,1,2); 

	field(0,2,0) = C3DFVector(0,3,1); 
	field(1,2,0) = C3DFVector(1,2,2); 
	field(2,2,0) = C3DFVector(2,1,2); 

	field(0,0,1) = C3DFVector(2,1,2); 
	field(1,0,1) = C3DFVector(3,1,4); 
	field(2,0,1) = C3DFVector(2,4,2); 

	field(0,1,1) = C3DFVector(5,6,7); 
	field(1,1,1) = C3DFVector(2,3,6); 
	field(2,1,1) = C3DFVector(3,2,4); 

	field(0,2,1) = C3DFVector(1,2,3); 
	field(1,2,1) = C3DFVector(2,3,8); 
	field(2,2,1) = C3DFVector(8,5,3); 

	field(0,0,2) = C3DFVector(2,3,5); 
	field(1,0,2) = C3DFVector(4,5,6); 
	field(2,0,2) = C3DFVector(4,3,1); 

	field(0,1,2) = C3DFVector(1,3,2); 
	field(1,1,2) = C3DFVector(7,8,9); 
	field(2,1,2) = C3DFVector(3,5,7); 

	field(0,2,2) = C3DFVector(1,2,5); 
	field(1,2,2) = C3DFVector(3,4,5); 
	field(2,2,2) = C3DFVector(4,5,2); 

}
