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
#include <mia/3d/maskedcost/mi.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace NS;

class MIFixture {
protected: 
	MIFixture(); 
	
	
	C3DBounds size; 
	P3DImage src; 
	P3DImage ref; 
	C3DBitImage mask; 
	C3DFVectorfield grad; 
	unique_ptr<C3DMIMaskedImageCost> cost; 
}; 


BOOST_FIXTURE_TEST_CASE( test_MI_3D_self, MIFixture )
{
	cost->set_reference(*ref);
	
	const double test_cost_value =-1.2440862175787268; 

	double cost_value = cost->value(*ref, mask);
	BOOST_CHECK_CLOSE(cost_value, test_cost_value, 0.1);

	C3DFVectorfield force(C3DBounds(4,4,4));
	
	BOOST_CHECK_CLOSE(cost->evaluate_force(*ref, mask, force), 1.0 * test_cost_value, 0.1);

	// unfortunately the gradient self tests fail because the 
	// spline based gradent evaluation is not zero at the expected minimum
}

/*
   currently all tests fail, because the test values are not yet evaluated


BOOST_FIXTURE_TEST_CASE( test_MI_3D, MIFixture )
{
	cost->set_reference(*ref);
	
	double cost_value = cost->value(*src);
	BOOST_CHECK_CLOSE(cost_value, -74.402 / 64.0, 0.1);

	C3DFVectorfield force(C3DBounds(8,8));

	BOOST_CHECK_CLOSE(cost->evaluate_force(*src, 1.0, force), -74.402, 0.1);


	for (auto iforce = force.begin(), ig = grad.begin(); ig != grad.end(); ++ig, ++iforce) {
		BOOST_CHECK_CLOSE(iforce->x, ig->x, 0.1f);
		BOOST_CHECK_CLOSE(iforce->y, ig->y, 0.1f);
	}; 
}
*/

MIFixture::MIFixture():
	size(4,4,4),
	mask(size),
	grad(size) 
	
{
	const float src_data[64] = {           /*   0  1  2  3  4  5  6  7  8  9     */    
		1, 1, 2, 2, 2, 3, 4, 4,        /*1                                 0 */
		4, 4, 3, 3, 2, 2, 2, 1,        /*2     1  2  1  1     1            6 */
		2, 2, 3, 4, 5, 6, 7, 8,        /*3        1  2     1  1  1  1      7 */
		8, 7, 2, 8, 3, 4, 2, 2,        /*4     1     1  1     2  1  1      7 */ 
		3, 1, 3, 4, 5, 6, 7, 8,        /*5     1     2  1        2  1      7 */
		3, 4, 4, 5, 6, 4, 2, 2,        /*6        3     3     1            7 */
	        3, 2, 3, 4, 5, 3, 1, 4,        /*7     1  1  2  2  1               7 */
		5, 6, 7, 3, 2, 1, 2, 6         /*8     1  2  1        2  1         7 */
	};                                     /*      5  9  9  8  2  7  5  3        */
	
	const float ref_data[64] = {
		1, 1, 1, 5, 1, 1, 1, 1, 
		2, 2, 2, 7, 2, 2, 2, 2,
		3, 3, 3, 5, 3, 3, 3, 3, 
		4, 4, 6, 4, 3, 4, 4, 4,
		5, 5, 5, 6, 4, 2, 1, 5, 
		6, 6, 4, 5, 6, 6, 6, 6,
	        7, 7, 7, 7, 7, 5, 7, 7, 
		8, 8, 8, 8, 8, 8, 8, 8
	};

	const float mask_data[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
	        0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1
	};


	const float grady[64] = {
		+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000,
		+0.00000, 0.00000,-1.30000, 0.00000,-0.35294,-0.35294,-0.35294, 0.00000,
		-0.00000,-0.70588, 0.68182, 0.00000, 0.16667, 3.76471, 0.00000, 0.00000,
		+0.00000,-1.66667,-0.00000, 0.00000,-0.00000, 0.00000,-0.00000,-0.00000,
		+0.00000,-0.00000,-1.36364,-0.00000, 0.50000, 0.00000, 0.00000, 0.00000,
		+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.70588, 0.00000,
		-0.00000,-0.23529,-2.04545, 0.00000,-0.66667, 2.04545, 0.00000,-0.00000,
		+0.00000,+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000
	}; 
	const float gradx[64] = {
		+ 0.00000,  0.00000, 0.00000, 0.00000, 0.00000, 0.00000,-0.00000,-0.00000, 
		+ 0.00000,- 0.00000, 1.30000,-0.00000, 0.11765,-0.00000, 0.11765,-0.00000,
		- 0.00000,- 0.11765,-1.36364, 0.00000, 0.33333, 3.76471, 1.40000, 0.00000, 
		+ 0.00000,-11.66667, 0.23529, 1.81818, 3.33333,-0.00000, 0.23529,-0.00000, 
		+ 0.00000,  0.00000,-2.04545, 0.00000, 0.33333, 0.40000, 0.00000, 0.00000,
	        - 0.00000,  0.00000, 0.00000, 0.33333,-0.20000, 0.00000, 0.23529,-0.00000,
		- 0.00000,- 0.35294,-1.36364,-0.00000,-0.16667, 2.72727, 0.00000,-0.00000,
		+ 0.00000,  0.00000,-0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000
	}; 

	const float gradz[64] = {
		+ 0.00000,  0.00000, 0.00000, 0.00000, 0.00000, 0.00000,-0.00000,-0.00000, 
		+ 0.00000,- 0.00000, 1.30000,-0.00000, 0.11765,-0.00000, 0.11765,-0.00000,
		- 0.00000,- 0.11765,-1.36364, 0.00000, 0.33333, 3.76471, 1.40000, 0.00000, 
		+ 0.00000,-11.66667, 0.23529, 1.81818, 3.33333,-0.00000, 0.23529,-0.00000, 
		+ 0.00000,  0.00000,-2.04545, 0.00000, 0.33333, 0.40000, 0.00000, 0.00000,
	        - 0.00000,  0.00000, 0.00000, 0.33333,-0.20000, 0.00000, 0.23529,-0.00000,
		- 0.00000,- 0.35294,-1.36364,-0.00000,-0.16667, 2.72727, 0.00000,-0.00000,
		+ 0.00000,  0.00000,-0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000
	}; 

	copy(mask_data, mask_data + 64, mask.begin()); 
	src.reset(new C3DFImage(size, src_data ));
	ref.reset(new C3DFImage(size, ref_data ));
	

	auto ig = grad.begin(); 
	for (int i = 0; i < 64; ++i, ++ig) {
		ig->x = gradx[i]; 
		ig->y = grady[i];
		ig->z = gradz[i];
	}
	cost.reset(new 	C3DMIMaskedImageCost(8, produce_spline_kernel("bspline:d=0"), 
					     8, produce_spline_kernel("bspline:d=3"), 0)); 
}
