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

	double test_cost_pair; 
}; 


BOOST_FIXTURE_TEST_CASE( test_SSD_3D_self, SSDFixture )
{
	cost->set_reference(*ref);
	
	const double test_cost_value =-0.0; 

	double cost_value = cost->value(*ref, mask);
	BOOST_CHECK_CLOSE(cost_value, test_cost_value, 0.1);

	C3DFVectorfield force(C3DBounds(4,4,4));
	
	BOOST_CHECK_CLOSE(cost->evaluate_force(*ref, mask, force), 1.0 * test_cost_value, 0.1);
	
	for(auto g : force) {
		BOOST_CHECK_EQUAL(g, C3DFVector(0,0,0));
	}
}

BOOST_FIXTURE_TEST_CASE( test_SSD_3D_pair, SSDFixture )
{
	cost->set_reference(*ref);
	
	double cost_value = cost->value(*src, mask);
	BOOST_CHECK_CLOSE(cost_value, test_cost_pair, 0.1);

	C3DFVectorfield force(C3DBounds(4,4,4));
	
	BOOST_CHECK_CLOSE(cost->evaluate_force(*src, mask, force), test_cost_pair, 0.1);

	for(auto iforce = force.begin(), iexpect = grad.begin(); iforce != force.end(); 
	    ++iforce, ++iexpect) {
		BOOST_CHECK_CLOSE(iforce->x, iexpect->x, 0.1);
		BOOST_CHECK_CLOSE(iforce->y, iexpect->y, 0.1);
		BOOST_CHECK_CLOSE(iforce->z, iexpect->z, 0.1);
	} 
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
	auto src_f = new C3DFImage(size, src_data ); 
	src.reset(src_f);
	auto ref_f = new C3DFImage(size, ref_data ); 
	ref.reset(ref_f);
	

	grad = get_gradient(*src_f); 

	auto ig = grad.begin(); 
	auto is = src_f->begin(); 
	auto ir = ref_f->begin(); 
	auto im = mask.begin(); 
	auto em = mask.end(); 

	test_cost_pair = 0.0; 
	while(im != em) {
		double delta = (*is - *ir); 
		*ig *= *im * delta / 64.0; 
		if (*im) 
			test_cost_pair += delta * delta; 

		++ig; ++is; ++ir; ++im; 
	}
	test_cost_pair /= 128.0; 
	
	cost.reset(new 	C3DSSDMaskedImageCost); 
}
