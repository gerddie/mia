/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/3d/filter/msnormalizer.hh>
#include <mia/3d/imagetest.hh>

using namespace msnormalizer_3dimage_filter; 
using namespace mia; 


BOOST_AUTO_TEST_CASE( test_n_elements )
{

	int n = 7; 
	int w = 3; 

	BOOST_CHECK_EQUAL(n_elements(0, n, w), 4); 
	BOOST_CHECK_EQUAL(n_elements(1, n, w), 5); 
	BOOST_CHECK_EQUAL(n_elements(2, n, w), 6); 
	BOOST_CHECK_EQUAL(n_elements(3, n, w), 7); 
	BOOST_CHECK_EQUAL(n_elements(4, n, w), 6); 
	BOOST_CHECK_EQUAL(n_elements(5, n, w), 5); 
	BOOST_CHECK_EQUAL(n_elements(6, n, w), 4); 



}

BOOST_AUTO_TEST_CASE( test_msnormalizer_from_plugin_real)
{
        C3DFImage image(C3DBounds(3,4,6)); 
        C3DFImage expect(C3DBounds(3,4,6)); 
	
	float init[72] = {
		1, 2, 3,  2, 3, 1,  2, 2, 5,  3, 3, 1,  
		3, 2, 3,  3, 3, 2,  2, 5, 5,  3, 5, 3,  
		4, 3, 4,  2, 3, 2,  2, 5, 2,  6, 5, 4,  
		5, 4, 3,  2, 3, 1,  2, 2, 5,  5, 6, 2,  
		4, 2, 4,  2, 4, 1,  5, 2, 2,  3, 3, 1,  
		1, 2, 3,  2, 3, 1,  2, 3, 2,  4, 3, 1
	}; 

	float test_data[72] = {
		
		-1.8480591388, -0.4281744193, 0.8400268813, -0.5, 0.2263120871, -1.4832396974, 
		-0.9574271078, -0.7237994818, 1.2001983963, -0.1002869446, -0.1757807623, -1.6426888243, 
		0.5254563899, -0.6493281021, 0.5254563899, 0.2586252121, 0.1572154843, -0.8384379738, 
		-0.9681718242, 1.35310631, 1.1899285569, -0.3875860749, 0.9967479589, -0.5051283393, 
		1.0181376927, 0.115334446, 1.443375673, -0.9509794538, -0.0310627228, -0.9344333141, 
		-1.0344111478, 1.1158901057, -0.9967479589, 1.2110601416, 0.7558641345, -0.0577350269, 
		1.7801819062, 0.9085875536, 0.1495249932, -0.9394989938, 0.0295071476, -1.5339391607, 
		-0.9605246372, -0.6664051775, 1.3245128873, 0.7104094931, 1.5461189428, -0.7537783614, 
		0.9775856785, -0.4915303319, 1.2165448525, -0.6670671874, 1.0749676998, -1.4073885704, 
		1.5339391607, -0.4807401701, -0.3509820591, -0.2431867286, 0.0376857693, -1.1129864055, 
		-1.40312152, -0.3578073095, 0.4183300133, -0.5773502692, 0.4346134937, -1.4220635116, 
		-1.0488088482, 0.4835011658, -0.1618347187, 0.8829187134, 0.3578073095, -1.3480755514
	}; 

	copy(init, init + 72, image.begin()); 
	copy(test_data, test_data + 72, expect.begin()); 

        auto c = BOOST_TEST_create_from_plugin<C3DMSNormalizerFilterPlugin>("msnormalizer:w=1");

        auto result = c->filter(image); 
        BOOST_REQUIRE(result); 

        test_image_equal(*result, expect);
        
}
