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
#include <mia/3d/maskedcost/ssd.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace NS;


class SSDFixture {
protected: 
	SSDFixture(); 
	
	
	C3DBounds size; 
	P3DImage src; 
	P3DImage ref; 
	C3DBitImage mask; 
	C3DFVectorfield grad; 
	unique_ptr<C3DSSDMaskedImageCost> cost; 
}; 


BOOST_FIXTURE_TEST_CASE( test_SSD_3D_self, SSDFixture )
{
	cost->set_reference(*ref);
	
	const double test_cost_value =-0.0; 

	double cost_value = cost->value(*ref, mask);
	BOOST_CHECK_CLOSE(cost_value, test_cost_value, 0.1);

	C3DFVectorfield force(C3DBounds(4,4,4));
	
	BOOST_CHECK_CLOSE(cost->evaluate_force(*ref, mask, force), 1.0 * test_cost_value, 0.1);
	
	BOOST_CHECK_EQUAL(force(0,0,0).x, 0.0f);
	BOOST_CHECK_EQUAL(force(0,0,0).y, 0.0f);
	BOOST_CHECK_EQUAL(force(0,0,0).z, 0.0f);

	BOOST_CHECK_EQUAL(force(1,1,1).x, 0.0f);
	BOOST_CHECK_EQUAL(force(1,1,1).y, 0.0f);
	BOOST_CHECK_EQUAL(force(1,1,1).z, 0.0f);

	BOOST_CHECK_EQUAL(force(2,2,2).x, 0.0f);
	BOOST_CHECK_EQUAL(force(2,2,2).y, 0.0f);
	BOOST_CHECK_EQUAL(force(2,2,2).z, 0.0f);
	
}

SSDFixture::SSDFixture():
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


	

	copy(mask_data, mask_data + 64, mask.begin()); 
	src.reset(new C3DFImage(size, src_data ));
	ref.reset(new C3DFImage(size, ref_data ));
	

	auto ig = grad.begin(); 
	for (int i = 0; i < 64; ++i, ++ig) {
		assert(0); 
	}
	cost.reset(new 	C3DSSDMaskedImageCost); 
}
