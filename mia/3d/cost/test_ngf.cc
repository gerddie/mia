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

	BOOST_CHECK_CLOSE( scalar.cost(field(0,0,0), field(1,1,1)), - 26.0f*26.0f, 0.01f); 
	
	double cost = 0.0; 
	auto grad = scalar.grad(3, 9, field.begin_range(C3DBounds(1,1,1), C3DBounds(3,3,3)), field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, - 26.0f* 26.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.x, 13.0f * 19.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.y, -13.0f * 15.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.z, -13.0f * 31.0f, 0.01f); 
	
}

BOOST_FIXTURE_TEST_CASE( test_ngf_evaluator_cross, C3DFVectorfieldFixture )
{
	
	C3DFVectorfield input(C3DBounds(3,3,3)); 
	
	FCross costfunct; 

	BOOST_CHECK_CLOSE( costfunct.cost(field(0,0,0), field(1,1,1)),10.0f, 0.01f); 
	
	double cost = 0.0; 
	auto grad = costfunct.grad(3, 9, field.begin_range(C3DBounds(1,1,1), C3DBounds(3,3,3)), field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, 10.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.x, 9.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.y, 1.0f, 0.01f); 
	BOOST_CHECK_CLOSE( grad.z, -11.0f, 0.01f); 
	
}

BOOST_FIXTURE_TEST_CASE( test_ngf_evaluator_delta_scalar, C3DFVectorfieldFixture )
{
	
	C3DFVectorfield input(C3DBounds(3,3,3)); 
	
	FDeltaScalar costfunct; 

	BOOST_CHECK_CLOSE( costfunct.cost(field(1,1,1),field(0,0,0)), 2*5.3330572096f, 0.01f); 
	
	double cost = 0.0; 
	auto ifield = field.begin_range(C3DBounds(1,1,1), C3DBounds(3,3,3)); 
	cvdebug() << "ifield:boundary=" << ifield.get_boundary_flags() << "\n"; 
	auto grad = costfunct.grad(3, 9, ifield, field(0,0,0), cost); 

	BOOST_CHECK_CLOSE( cost, 2*5.3330572096f, 0.01f); 

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


class FEvaluatorMock: public FEvaluator {
public:
	double cost (const C3DFVector& /*src*/, const C3DFVector& /*ref*/) const {
		return 1.0; 
	}
	
	C3DFVector grad(int /*nx*/, int /*nxy*/, C3DFVectorfield::const_range_iterator irsrc,
			const C3DFVector& /*ref*/, double& cost) const {
		cost += 1; 
		C3DFVector result; 
		if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_x))
			result.x = 1;
		
		if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_y))
			result.y = 2;
		
		if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_z))
			result.z = 3; 
		
		return result; 
	}
};




BOOST_AUTO_TEST_CASE( test_ngf_field ) 
{
	
	C3DNFGImageCost cost(PEvaluator(new FEvaluatorMock));

	C3DBounds size(3,4,5); 
	
	C3DFImage test(size); 
	C3DFImage ref(size); 
	C3DFVectorfield field(size);  
	fill(field.begin(), field.end(), C3DFVector::_0); 

	cost.set_reference(ref); 
	
	// the boundary should be ignored 
	BOOST_CHECK_EQUAL(cost.value(test), 30); 

	double value = cost.evaluate_force(test, 10, field);
	BOOST_CHECK_EQUAL(value, 30);
	
	C3DFVector testv; 
	auto ifield = field.begin(); 
	for (unsigned z = 0; z < size.z; ++z) {
		testv.z = (z == 0 || z == size.z -1) ? 0 : 3; 
		for (unsigned y = 0; y < size.y; ++y) {
			testv.y = (y == 0 || y == size.y -1) ? 0 : 2; 
			for (unsigned x = 0; x < size.x; ++x, ++ifield) {
				testv.x = (x == 0 || x == size.x -1) ? 0 : 1;
				BOOST_CHECK_EQUAL(*ifield, testv); 
			}
		}
	}
	
}
