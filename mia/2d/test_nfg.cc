/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>


#include <mia/2d/nfg.hh>



NS_MIA_USE
using namespace boost;
using namespace std;

P2DImage create_test_image()
{
	const float init_data[9] = { 0, 1, 0,  6, 4, 0,  4, 9, 4 };
	C2DBounds size(3,3);
	return P2DImage(new C2DFImage(size, init_data));
}

BOOST_AUTO_TEST_CASE(check_nfg_n )
{
	P2DImage image = create_test_image();

	const float noise_level = 9.0 / 5.0;
	C2DFVectorfield ngf = get_nfg_n(*image,  noise_level);


	C2DFVector center = ngf(1,1);

	BOOST_CHECK_CLOSE(center.x , -3.0f / sqrtf(26.f), 0.1);
	BOOST_CHECK_CLOSE(center.y , 4.0f / sqrtf(26.f), 0.1);

	C2DFVectorfield ngf2 = get_nfg_n(*image,  0.0);
	center = ngf2(1,1);

	BOOST_CHECK_CLOSE(center.x , -3.0f / 5.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y , 4.0f / 5.0f, 0.1);
}

BOOST_AUTO_TEST_CASE(check_nfg_j)
{
	P2DImage image = create_test_image();

	const float jump_level = get_jump_level(*image, 9.0 / 5.0);
	C2DFVectorfield ngf = get_nfg_j(*image,  jump_level*jump_level);

	C2DFVector center = ngf(1,1);

	BOOST_CHECK_CLOSE(center.x , -3.0f / sqrtf(26.f), 0.1);
	BOOST_CHECK_CLOSE(center.y , 4.0f / sqrtf(26.f), 0.1);

	C2DFVectorfield ngf2 = get_nfg_j(*image,  0.0);
	center = ngf2(1,1);

	BOOST_CHECK_CLOSE(center.x , -3.0f / 5.0f, 0.1);
	BOOST_CHECK_CLOSE(center.y , 4.0f / 5.0f, 0.1);
}

BOOST_AUTO_TEST_CASE(check_intensity_jump_level)
{
	P2DImage image = create_test_image();
	const float noise_level = 9.0 / 5.0;

	BOOST_CHECK_CLOSE(get_jump_level(*image, noise_level), 1.0f, 0.1);

}

BOOST_AUTO_TEST_CASE(test_get_jump_level2)
{
	P2DImage image = create_test_image();

	BOOST_CHECK_CLOSE(get_jump_level(*image), 1.94624734f * 5.0f / 9.0f, 0.1);
}

BOOST_AUTO_TEST_CASE( check_get_noise_level )
{
	P2DImage image = create_test_image();
	BOOST_CHECK_CLOSE(get_noise_level(*image),1.94624734f, 0.1);
}



