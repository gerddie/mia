/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/2d/filter/adaptmed.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace adaptmed_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_adaptmed )
{
	// there should really be a test here
	const size_t size_x = 7;
	const size_t size_y = 5;

	const int src[size_y*size_x] =
		{ 0, 1, 2, 3, 2, 3, 5,
		  2, 5, 2, 3, 5, 3, 2,
		  1, 2, 4, 4, 3, 2, 1,
		  3, 4, 4, 3, 4, 3, 2,
		  1, 3, 2, 4, 5, 6, 2};

	// "hand filtered" w = 1 -> 3x3
	const int src_ref[size_y*size_x] =
		{ 0, 2, 2, 3, 2, 3, 5,
		  1, 5, 3, 3, 5, 3, 2,
		  1, 3, 4, 4, 3, 3, 1,
		  2, 4, 3, 4, 4, 3, 2,
		  1, 3, 2, 4, 4, 6, 3};

	C2DBounds size(size_x, size_y);

	C2DSIImage *src_img = new C2DSIImage(size, src);


	C2DAdaptMedian admedian(2);

	P2DImage src_wrap(src_img);

	P2DImage res_wrap = admedian.filter(*src_wrap);


	C2DSIImage* res_img = dynamic_cast<C2DSIImage*>(&*res_wrap);

	BOOST_REQUIRE(res_img);
	BOOST_CHECK_EQUAL(res_img->get_size(), src_img->get_size());

	size_t j = 0;
	for (C2DSIImage::const_iterator i = res_img->begin();
	     i != res_img->end(); ++i, ++j) {
		cvdebug() << j << " - \n";
		BOOST_CHECK_EQUAL(*i,  src_ref[j]);
	}
}

