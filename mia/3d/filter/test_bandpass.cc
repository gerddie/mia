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
#include <mia/3d/filter/bandpass.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;

BOOST_AUTO_TEST_CASE( test_bandpass )
{
	const float src_data[24] = { 1.0f,  2.0f, 2.1f, -1.0f, 2.0f,
				     11.0f, 21.0f, 3.1f, -4.0f, 4.0f,
				     5.0f,  1.0f, 6.1f,-21.2f,22.0f,
				     11.0f, 12.2f, 8.1f, -5.0f,24.0f,
				     1.3f,  2.3f, 9.1f, -6.2f};

	const float ref_data[25] = { 1.0f,  2.0f, 2.1f, -1.0f, 2.0f,
				     0.0f,  0.0f, 3.1f, -4.0f, 4.0f,
				     5.0f,  1.0f, 0.0f,  0.0f, 0.0f,
				     0.0f,  0.0f, 0.0f, -5.0f, 0.0f,
				     1.3f,  2.3f, 0.0f,  0.0f, 2.2f};
	const float min = -6.0;
	const float max =  5.0;

	C3DFImage *src = new C3DFImage(C3DBounds(2, 3, 4));

	C3DFImage::iterator f = src->begin();
	for (size_t i = 0; i < 24; ++i, ++f)
		*f = src_data[i];

	std::shared_ptr<C3DImage > srcw(src);

	C3DImageBandPass filter(min, max);

	std::shared_ptr<C3DImage > res = filter.filter(*srcw);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);

	C3DFImage *resi = dynamic_cast<C3DFImage *>(res.get());
	BOOST_REQUIRE(resi);

	BOOST_CHECK_EQUAL(resi->get_size(), src->get_size());

	f = resi->begin();
	for (size_t i = 0; i < 24; ++i, ++f)
		BOOST_CHECK_CLOSE(*f, ref_data[i], 0.1);
}
