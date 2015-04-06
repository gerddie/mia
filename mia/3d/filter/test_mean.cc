/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


/*
  The test result numbes were created with libreoffice' calc program 4.1.5.3 
  testhelpers/3d-mean-variance-test.ods
*/


#include <mia/internal/plugintester.hh>
#include <mia/3d/filter/mean.hh>
#include <mia/3d/imagetest.hh>


using namespace mia;
using namespace std;
using namespace mean_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_mean_from_plugin_simple)
{
        C3DFImage image(C3DBounds(3,4,5)); 
	

        fill(image.begin(), image.end(), 1.0f); 

        auto c = BOOST_TEST_create_from_plugin<C3DMeanFilterPlugin>("mean:w=1");

        auto result = c->filter(image); 
        BOOST_REQUIRE(result); 

        test_image_equal(*result, image);
        
}

BOOST_AUTO_TEST_CASE( test_mean_from_plugin_real)
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
		19.0f/ 8, 28.0f/12, 19.0f/ 8,   30.0f/12, 49.0f/18, 36.0f/12,   36.0f/12, 53.0f/18, 38.0f/12,   25.0f/ 8, 39.0f/12, 29.0f/8, 
		31.0f/12, 46.0f/18, 31.0f/12,   49.0f/18, 76.0f/27, 55.0f/18,   59.0f/18, 84.0f/27, 59.0f/18,   43.0f/12, 63.0f/18, 45.0f/12,
		37.0f/12, 52.0f/18, 33.0f/12,   55.0f/18, 82.0f/27, 57.0f/18,   64.0f/18, 90.0f/27, 63.0f/18,   48.0f/12, 69.0f/18, 49.0f/12,
		38.0f/12, 53.0f/18, 34.0f/12,   56.0f/18, 80.0f/27, 52.0f/18,   62.0f/18, 82.0f/27, 53.0f/18,   46.0f/12, 62.0f/18, 39.0f/12,
		34.0f/12, 47.0f/18, 31.0f/12,   50.0f/18, 72.0f/27, 47.0f/18,   56.0f/18, 72.0f/27, 45.0f/18,   40.0f/12, 53.0f/18, 32.0f/12,
		20.0f/ 8, 29.0f/12, 20.0f/ 8,   32.0f/12, 45.0f/18, 29.0f/12,   36.0f/12, 44.0f/18, 26.0f/12,   25.0f/ 8, 31.0f/12, 17.0f/8
	}; 

	copy(init, init + 72, image.begin()); 
	copy(test_data, test_data + 72, expect.begin()); 

        auto c = BOOST_TEST_create_from_plugin<C3DMeanFilterPlugin>("mean:w=1");

        auto result = c->filter(image); 
        BOOST_REQUIRE(result); 

        test_image_equal(*result, expect);
        
}


BOOST_AUTO_TEST_CASE( test_variance_from_plugin_real)
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
		0.6461538669, 0.8842581575, 0.9418553707, 0.9541867709, 1.1381303158, 1.2430416439, 
		0.9629315991, 1.3082114628, 1.5308439032, 1.1808589713, 1.4629136824, 1.7135035969,
		0.6667967162, 0.8908964824, 0.9118896119, 1.0053070916, 1.1495839577, 1.2117508995, 
		1.1343259506, 1.3065862705, 1.404543673, 1.3591319872, 1.4624087158, 1.5652038894, 
		0.8672395154, 0.9725722993, 0.9184631921, 1.1332712116, 1.2260257661, 1.2400617594, 
		1.3691203891, 1.3795603149, 1.4392941391, 1.634941698, 1.5525440296, 1.627502342, 
		1.035581741, 1.1611008606, 1.1442518335, 1.1923399406, 1.2712151864, 1.2229523468, 
		1.3388592824, 1.3737998014, 1.3841640767, 1.5296769137, 1.4715249579, 1.4543121033, 
		1.0848790377, 1.1732768267, 1.1340058468, 1.122744563, 1.1830172368, 1.0975180696, 
		1.1308862301, 1.2406709754, 1.3025937332, 1.2587245411, 1.2727286891, 1.3019245601, 
		1.0166710035, 1.1319696192, 1.1609364188, 1.0684026803, 1.0763137756, 0.9753916592, 
		0.92694914, 1.0396658344, 1.0392902185, 0.9542701996, 0.9860666514, 0.8661049513
	}; 

	copy(init, init + 72, image.begin()); 
	copy(test_data, test_data + 72, expect.begin()); 

        auto c = BOOST_TEST_create_from_plugin<C3DVarianceFilterPlugin>("variance:w=1");

        auto result = c->filter(image); 
        BOOST_REQUIRE(result); 

        test_image_equal(*result, expect);
        
}
