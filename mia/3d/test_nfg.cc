/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#include <climits>
#include <mia/internal/autotest.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/nfg.hh>


NS_MIA_USE
using namespace boost;
using namespace std;

const float check_noise_level = sqrt((499.0 - 131.0 * 131.0 / 54.0) / 53.0 );

P3DImage create_test_image()
{
	const float init_data[27] = { 0, 1, 0,
				      6, 4, 0,
				      4, 9, 4,

				      1, 2, 1,
				      5, 5, 5,
				      6, 8, 6,

				      0, 1, 0,
				      7, 3, 0,
				      2, 7, 6
	};

	/*
	  0:  5 :   0 :   0
	  1: 19 :  19 :  19
	  2:  9 :  18 :  36
	  3:  4 :  12 :  36
	  4:  7 :  28 : 112
	  5:  7 :  35 : 175
	  6:  2 :  12 :  72
	  7:  1 :   7 :  49
	Sum: 54 : 131 : 499
	 */

	C3DBounds size(3,3,3);
	return P3DImage(new C3DFImage(size, init_data));
}


BOOST_AUTO_TEST_CASE( check_nfg_n )
{
	P3DImage image = create_test_image();

	const float noise_level = 27.0 / sqrt(9.25);
	C3DFVectorfield ngf = get_nfg_n(*image,  noise_level);


	C3DFVector center = ngf(1,1,1);

	BOOST_CHECK_CLOSE(center.x , 0.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y , 3.0f / sqrtf(10.25f), 0.1);
	BOOST_CHECK_CLOSE(center.z , -0.5f / sqrtf(10.25f), 0.1);

	C3DFVectorfield ngf2 = get_nfg_n(*image,  0.0);
	center = ngf2(1,1,1);


	BOOST_CHECK_CLOSE(center.x ,  0.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y ,  3.0f / sqrt(9.25f), 0.1);
	BOOST_CHECK_CLOSE(center.z ,  -0.5f / sqrt(9.25f), 0.1);
}


BOOST_AUTO_TEST_CASE( check_nfg_j )
{
	P3DImage image = create_test_image();

	const float jump_level = get_jump_level(*image, 27.0 / sqrt(9.25));
	C3DFVectorfield ngf = get_nfg_j(*image,  jump_level*jump_level);

	C3DFVector center = ngf(1,1,1);

	BOOST_CHECK_CLOSE(center.x , 0.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y , 3.0f / sqrtf(10.25f), 0.1);
	BOOST_CHECK_CLOSE(center.z , -0.5f / sqrtf(10.25f), 0.1);


	C3DFVectorfield ngf2 = get_nfg_j(*image,  0.0);
	center = ngf2(1,1,1);

	BOOST_CHECK_CLOSE(center.x ,  0.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y ,  3.0f / sqrt(9.25f), 0.1);
	BOOST_CHECK_CLOSE(center.z ,  -0.5f / sqrt(9.25f), 0.1);
}



BOOST_AUTO_TEST_CASE( check_get_noise_level )
{

	P3DImage image = create_test_image();
	BOOST_CHECK_CLOSE(get_noise_level(*image),check_noise_level, 0.1);
}


BOOST_AUTO_TEST_CASE( check_intensity_jump_level )
{
	P3DImage image = create_test_image();
	const float noise_level = 27.0 / sqrt(9.25);

	BOOST_CHECK_CLOSE(get_jump_level(*image, noise_level), 1.0f, 0.1);

}

BOOST_AUTO_TEST_CASE(  test_get_jump_level2 )
{
	P3DImage image = create_test_image();
	BOOST_CHECK_CLOSE(get_jump_level(*image), check_noise_level * sqrt(9.25f) / 27.0f , 0.1);
}

