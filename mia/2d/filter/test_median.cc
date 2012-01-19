/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/median.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace median_2dimage_filter;


BOOST_AUTO_TEST_CASE( test_2dfilter_median )
{
	const size_t size_x = 7;
	const size_t size_y = 5;

	const int src[size_y][size_x] =
		{{ 0, 1, 2, 3, 2, 3, 5},
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 1, 2, 7, 6, 4, 2, 1},
		 { 3, 4, 4, 3, 4, 3, 2},
		 { 1, 3, 2, 4, 5, 6, 2}};

	// "hand filtered" w = 1 -> 3x3
	const int src_ref[size_y][size_x] =
		{{ 1, 2, 2, 2, 3, 3, 3},
		 { 1, 2, 3, 3, 3, 3, 2},
		 { 2, 3, 4, 4, 3, 3, 2},
		 { 2, 3, 4, 4, 4, 3, 2},
		 { 3, 3, 3, 4, 4, 3, 2}};


	C2DBounds size(size_x, size_y);

	C2DSIImage *src_img = new C2DSIImage(size);
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x)
			(*src_img)(x,y) = src[y][x];

	auto median = BOOST_TEST_create_from_plugin<C2DMedianFilterPluginFactory>("median:w=1"); 

	P2DImage src_wrap(src_img);

	P2DImage res_wrap = median->filter(*src_wrap);

	C2DSIImage* res_img = dynamic_cast<C2DSIImage*>(res_wrap.get());
	BOOST_REQUIRE(res_img);
	BOOST_REQUIRE(res_img->get_size() == src_img->get_size());


	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x)
			BOOST_CHECK_EQUAL((*res_img)(x,y), src_ref[y][x]);
}

BOOST_AUTO_TEST_CASE( test_2dfilter_saltnpepper )
{
	float input_data[9] = {
		-1000, 2, 3,
		4, 5, 6,
		7, 8, 9,
	};

	float test_data[9] = {
		3, 2, 3,
		4, 5, 6,
		7, 8, 9,
	};

	C2DFImage input(C2DBounds(3,3));

	copy(input_data, input_data + input.size(), input.begin());
	auto filter = BOOST_TEST_create_from_plugin<C2DSaltAndPepperFilterFactory>("sandp:w=1,thresh=1000"); 

	P2DImage result = filter->filter(input);
	BOOST_REQUIRE(result->get_size() == input.get_size());

	C2DFImage *presult = dynamic_cast<C2DFImage *>(result.get());
	BOOST_REQUIRE(presult);

	size_t k = 0;

	for (C2DFImage::const_iterator i = presult->begin(); i != presult->end(); ++i, ++k) {
		BOOST_CHECK_EQUAL(*i, test_data[k]);
	}
}


