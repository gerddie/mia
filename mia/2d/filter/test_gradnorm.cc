/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/2d/filter/gradnorm.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace gradnorm_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_gradnorm )
{
	const size_t size = 4;
	const float src_data[size * size] = {
		1, 1, 1, 1,
		2, 3, 4, 2,
		3, 4, 2, 1,
		2, 4, 1, 2,
	};

	const float ref_data[size * size] = {
		0, 0, 0, 0,
		sqrt(4.0f/18.0f), sqrt(13.0f/18.0f), sqrt(2.0f/18.0f), 0,
		0, sqrt(2.0f/18.0f), 1, 0,
		0, sqrt(1.0f/18.0f), sqrt(4.0f/18.0f), 0,
	};


	C2DFImage *src = new C2DFImage(C2DBounds(size, size));

	C2DFImage::iterator f = src->begin();
	for (size_t i = 0; i < size*size; ++i, ++f)
		*f = src_data[i];

	P2DImage srcw(src);

	auto filter = BOOST_TEST_create_from_plugin<C2DGradnormFilterPlugin>("gradnorm:normalize=1");	

	P2DImage res = filter->filter(*srcw);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);
	BOOST_REQUIRE(res->get_pixel_type() == it_float);

	C2DFImage *resi = dynamic_cast<C2DFImage *>(res.get());
	BOOST_REQUIRE(resi);

	BOOST_CHECK_EQUAL(resi->get_size(), src->get_size());
	BOOST_REQUIRE(resi->get_size() == src->get_size());

	f = resi->begin();
	for (size_t i = 0; i < size*size; ++i, ++f)
		BOOST_CHECK_CLOSE(1.0 + *f, 1.0 + ref_data[i], 0.1);
}


BOOST_AUTO_TEST_CASE( test_gradnorm_non_normalized )
{
	const size_t size = 4;
	const float src_data[size * size] = {
		1, 1, 1, 1,
		2, 3, 4, 2,
		3, 4, 2, 1,
		2, 4, 1, 2,
	};

	const float ref_data[size * size] = {
		0, 0, 0, 0,
		1.0f, sqrt(3.25f), sqrt(0.5f), 0,
		0, sqrt(0.5f), sqrt(4.5f), 0,
		0, 0.5f, 1.0f, 0,
	};

	C2DFImage *src = new C2DFImage(C2DBounds(size, size));

	C2DFImage::iterator f = src->begin();
	for (size_t i = 0; i < size*size; ++i, ++f)
		*f = src_data[i];

	P2DImage srcw(src);

	auto filter = BOOST_TEST_create_from_plugin<C2DGradnormFilterPlugin>("gradnorm:normalize=0");	

	P2DImage res = filter->filter(*srcw);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);
	BOOST_REQUIRE(res->get_pixel_type() == it_float);

	C2DFImage *resi = dynamic_cast<C2DFImage *>(res.get());
	BOOST_REQUIRE(resi);

	BOOST_CHECK_EQUAL(resi->get_size(), src->get_size());
	BOOST_REQUIRE(resi->get_size() == src->get_size());

	f = resi->begin();
	for (size_t i = 0; i < size*size; ++i, ++f)
		BOOST_CHECK_CLOSE(1.0 + *f, 1.0 + ref_data[i], 0.1);
}
