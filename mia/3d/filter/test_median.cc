/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#include <mia/core/shared_ptr.hh>
#include <mia/internal/autotest.hh>
#include <mia/3d/filter/median.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace median_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_median )
{
	float input_data[27] = {
		1, 2, 3,
		4, 5, 6,
		7, 8, 9,

		11, 12, 13,
		14, 15, 16,
		17, 18, 19,

		21, 22, 23,
		24, 25, 26,
		27, 28, 29,

	};

	float test_data[27] = {
		8, 8.5,    9,
		9.5,  10, 10.5,
		11, 11.5,  12,

		13,  13.5, 14,
		14.5, 15, 15.5,
		16, 16.5, 17,

		18, 18.5, 19,
		19.5, 20, 20.5,
		21, 21.5, 22
	};

	C3DFImage input(C3DBounds(3,3,3));

	copy(input_data, input_data + input.size(), input.begin());
	C3DMedianFilter filter(1);

	P3DImage result = filter.filter(input);
	BOOST_CHECK_EQUAL(result->get_size(), input.get_size());


	const C3DFImage& presult = dynamic_cast<const C3DFImage&>(*result);

	size_t k = 0;
	for (C3DFImage::const_iterator i = presult.begin(); i != presult.end(); ++i, ++k)
		BOOST_CHECK_CLOSE(*i, test_data[k], 0.1);
}

BOOST_AUTO_TEST_CASE( test_saltnpepper )
{
	float input_data[27] = {
		-1000, 2, 3,
		4, 5, 6,
		7, 8, 9,

		11, 12, 13,
		14, 2000, 16,
		17, 18, 19,

		21, 22, 23,
		24, 25, 26,
		27, 28, 29,

	};

	float test_data[27] = {
		8, 2, 3,
		4, 5, 6,
		7, 8, 9,

		11, 12, 13,
		14, 16, 16,
		17, 18, 19,

		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
	};

	C3DFImage input(C3DBounds(3,3,3));

	copy(input_data, input_data + input.size(), input.begin());
	C3DSaltAndPepperFilter filter(1, 1000);

	P3DImage result = filter.filter(input);
	BOOST_CHECK_EQUAL(result->get_size(), input.get_size());

	const C3DFImage& presult = dynamic_cast<const C3DFImage&>(*result);

	size_t k = 0;

	for (C3DFImage::const_iterator i = presult.begin(); i != presult.end(); ++i, ++k)
		BOOST_CHECK_CLOSE(*i, test_data[k], 0.1);

}
