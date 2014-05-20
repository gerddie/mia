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


#include <mia/internal/plugintester.hh>
#include <mia/3d/maskedcost/lncc.hh>

using namespace NS; 
using namespace mia; 




BOOST_AUTO_TEST_CASE( test_masked_lncc_1 ) 
{
        C3DBounds size(4,4,4); 
        auto lncc = BOOST_TEST_create_from_plugin<CLNCC3DImageCostPlugin>("lncc:w=1");

        	const float src_data[64] = {           /*   0  1  2  3  4  5  6  7  8  9     */    
		1, 1, 2, 2, 
		2, 3, 4, 4,        /*1                                 0 */
		4, 4, 3, 3, 
		2, 2, 2, 1,        /*2     1  2  1  1     1            6 */
		
		2, 2, 3, 4, 
		5, 6, 7, 8,        /*3        1  2     1  1  1  1      7 */
		8, 7, 2, 8, 
		3, 4, 2, 2,        /*4     1     1  1     2  1  1      7 */ 
		
		3, 1, 3, 4, 
		5, 6, 7, 8,        /*5     1     2  1        2  1      7 */
		3, 4, 4, 5, 
		6, 4, 2, 2,        /*6        3     3     1            7 */
		
	        3, 2, 3, 4, 
		5, 3, 1, 4,        /*7     1  1  2  2  1               7 */
		5, 6, 7, 3, 
		2, 1, 2, 6         /*8     1  2  1        2  1         7 */
		};                                     /*      5  9  9  8  2  7  5  3        */
	
	const float ref_data[64] = {
		1, 1, 1, 5, 
		1, 1, 1, 1, 
		2, 2, 2, 7,
		2, 2, 2, 2,
		
		3, 3, 3, 5, 
		3, 3, 3, 3, 
		4, 4, 6, 4, 
		3, 4, 4, 4,

		5, 5, 5, 6, 
		4, 2, 1, 5, 
		6, 6, 4, 5, 
		6, 6, 6, 6,
		
	        7, 7, 7, 7, 
		7, 5, 7, 7, 
		8, 8, 8, 8, 
		8, 8, 8, 8
	};

	const bool mask_data[64] = {
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
	        0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1
	};

        
	C3DFImage src_f(size, src_data); 
        C3DFImage ref_f(size, ref_data); 
        C3DBitImage mask(size, mask_data); 

        
        lncc->set_reference(ref_f); 
        BOOST_CHECK_SMALL(lncc->value(ref_f, mask), 1e-10);
        
        auto v = lncc->value(src_f, mask); 
        BOOST_CHECK_CLOSE(v, 1.0 - 0.1206397729, 0.1); 
        
	C3DFVectorfield zero_force(size); 
	v = lncc->evaluate_force(ref_f, mask, zero_force); 
	BOOST_CHECK_SMALL(v,  1e-10);

	for (auto iv = zero_force.begin(); iv != zero_force.end();  ++iv) {
		BOOST_CHECK_SMALL(iv->x, 1e-8f); 
		BOOST_CHECK_SMALL(iv->y, 1e-8f); 
		BOOST_CHECK_SMALL(iv->z, 1e-8f); 
	}
	
	


	C3DFVectorfield force(size); 

	v = lncc->evaluate_force(src_f, mask, force); 
	BOOST_CHECK_CLOSE(v, 1.0 - 0.1206397729, 0.1); 
        
	
	float gradx[] = {
		0,0.0064692292,0.0067829344,0, 
		0,0.0211819153,0.0025820212,0, 
		0,-0.0034389918,-0.0007584461,0, 
		0,0,-0.0051883059,0, 
		
		0,0.0001722104,0,0, 
		0.,8.16718273792367E-005,-0.0002484197,0, 
		0,0.0008372734,0.0021445316, 0, 
		0,-0.0009001972,-0.0003612011,0, 
		
		0,0,-0.0051916234,0, 
		0,-0.0158841787,-0.0215016708,0, 
		0,0.0032574205,-0.0051260847,0, 
		0,-0.0004368596,0.0001938512,0, 
		
		0,0,0.0296423058,0, 
		0,0.0132227233,0.001550289,0, 
		0,0.0402855406,-0.0435344672,0, 
		0,0,-0.0193401882,0
	};

	float grady[] = {
		0,0,0,0,
		0.0676691729,0.0317728729,0.0025820212,-0.0056788204,
		0,-0.0034389918,-0.0015168922,-0.0150381199,
		0,0,0,0,
		0,0,0,0,
		0.0041567101,0.0002041796,0.0001242098,-0.0022727478,
		0,0.0002790911,-0.0107226582,-0.0106848188,
		0,0,0,0,
		0,0,0,0,
		0,-0.023826268,-0.0107508354,0.0073412295,
		0,-0.0065148409,0.0256304233,0.0056020016,
		0,0,0,0,
		0,0,0,0,
		0.015625,-0.0132227233,0.0062011562,-0.0092450912,
		0,-0.0402855406,0.0145114891,0.019820525,
		0,0,0,0
	}; 
	
	float gradz[] = {
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		
		0,0,0,0.003136997,
		0.002078355,0.0001225077,-0.0003726295,-0.0022727478,
		0,0,0.0021445316,0.0035616063,
		0.0282984061,0.0018003944,0,0.0004336735,
		
		0,0,0,0,
		0,0.023826268,0.0645050124,-0.0293649178,
		0,-0.0032574205,-0.0256304233,0.0046683347,
		-0.0065214016,-0.0003276447,0,-0.0010597761,
		
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0
		
	}; 

	auto igx = gradx; 
	auto igy = grady; 
	auto igz = gradz; 
	for (auto iv = force.begin(); iv != force.end();  ++iv, ++igx, ++igy, ++igz) {
		BOOST_CHECK_CLOSE(iv->x, *igx, 0.1); 
		BOOST_CHECK_CLOSE(iv->y, *igy, 0.1); 
		BOOST_CHECK_CLOSE(iv->z, *igz, 0.1); 
	}
}
