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
#include <mia/2d/maskedcost/lncc.hh>

using namespace NS; 
using namespace mia; 




BOOST_AUTO_TEST_CASE( test_masked_lncc_1 ) 
{
        C2DBounds size(4,4); 
        auto lncc = BOOST_TEST_create_from_plugin<CLNCC2DImageCostPlugin>("lncc:w=1");

	const float src_data[16] = {           
		1, 1, 2, 2, 
		2, 3, 4, 4, 
		4, 4, 3, 3, 
		2, 2, 2, 1, 
		
		};                                     /*      5  9  9  8  2  7  5  3        */
	
	const float ref_data[16] = {
		1, 2, 1, 5, 
		1, 1, 1, 1, 
		2, 2, 2, 7,
		4, 3, 2, 2
	};

	const bool mask_data[64] = {
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
	};

        
	C2DFImage src_f(size, src_data); 
        C2DFImage ref_f(size, ref_data); 
        C2DBitImage mask(size, mask_data); 

        
        lncc->set_reference(ref_f); 
        BOOST_CHECK_SMALL(lncc->value(ref_f, mask),1e-10);
        
        auto v = lncc->value(src_f, mask); 
        BOOST_CHECK_CLOSE(v, 1.0 -0.199159659, 0.1); 
        
	C2DFVectorfield zero_force(size); 
	v = lncc->evaluate_force(ref_f, mask, zero_force); 
	BOOST_CHECK_SMALL(v, 1e-10);

	for (auto iv = zero_force.begin(); iv != zero_force.end();  ++iv) {
		BOOST_CHECK_SMALL(iv->x, 1e-8f); 
		BOOST_CHECK_SMALL(iv->y, 1e-8f); 
	}
	

	C2DFVectorfield force(size); 

	v = lncc->evaluate_force(src_f, mask, force); 
	BOOST_CHECK_CLOSE(v, 1.0 -0.199159659, 0.1); 
        
	
	float gradx[] = {
		0,0.1423816568,-0.0565096061,0,
		0,-0.0094921105,-0.0110429448,0,
		0,-0.0464876033,0.0032142857,0,
		0,0,-0.0148760331,0
	};

	float grady[] = {
		0,0,0,0,
		0,-0.0142381657,-0.0110429448,-0.0253807107,
		0,-0.0464876033,0.0064285714,-0.1171571544,
		0,0,0,0
	}; 
	

	auto igx = gradx; 
	auto igy = grady; 
	for (auto iv = force.begin(); iv != force.end();  ++iv, ++igx, ++igy) {
		if (*igx == 0.0) 
			BOOST_CHECK_SMALL(iv->x, 1e-8f); 
		else 
			BOOST_CHECK_CLOSE(iv->x, *igx, 0.1); 

		if (*igy == 0.0) 
			BOOST_CHECK_SMALL(iv->y, 1e-8f); 
		else 
			BOOST_CHECK_CLOSE(iv->y, *igy, 0.1); 
	}
}
