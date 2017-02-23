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

#include <mia/2d/model/navier.hh>

NS_USE(navier_regmodel);
NS_MIA_USE;

using namespace std;

struct ModelFixture {

	ModelFixture();

	C2DNavierRegModel model;
};

ModelFixture::ModelFixture():
	model(1.0, 2, 100, 1.0, 0.0001)
{
}

BOOST_FIXTURE_TEST_CASE( test_navier_kernel, ModelFixture )
{
	// assume mu = 1, lambda = 2
	//	float a_b = 0.4f;
	//	float a   = 0.1f;
	//	float b_4 = 0.075;

	C2DFVectorfield v(C2DBounds(3,3));

	C2DFVector b(1.0, 2.0);

	v(0,0) = C2DFVector(2.0, 3.0);
	v(1,0) = C2DFVector(2.0, 4.0);
	v(2,0) = C2DFVector(0.0, 2.0);

	v(0,1) = C2DFVector(2.0, 1.0);
	v(1,1) = C2DFVector(0.5, 0.5);
	v(2,1) = C2DFVector(3.0, 3.0);

	v(0,2) = C2DFVector(1.0, 0.0);
	v(1,2) = C2DFVector(4.0, 4.0);
	v(2,2) = C2DFVector(3.0, 2.0);

	C2DFVector p( 3.6, 5.6 );
	C2DFVector q( 0.225, 0.3);

	C2DFVector test0(3.325, 5.4);
	C2DFVector test(3.825, 5.9);

	float res = model.solve_at(1, 1, b,v);
	BOOST_CHECK_CLOSE(res, test0.norm(), 0.01);

}


