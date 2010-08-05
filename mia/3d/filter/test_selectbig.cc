/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/3d/filter/selectbig.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace selectbig_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_selectbig )
{
	const unsigned short input_data[4*4*4] = {
		1, 2, 2, 3,  1, 1, 2, 2,  1, 1, 1, 2,  0, 1, 1, 1,
		1, 1, 3, 3,  1, 2, 2, 3,  1, 2, 2, 3,  4, 4, 4, 4,
		1, 3, 3, 3,  1, 3, 3, 3,  4, 4, 3, 3,  4, 4, 4, 5,
		1, 6, 6, 6,  1, 6, 6, 3,  4, 4, 5, 5,  7, 5, 5, 5
	};
/*
	long test_numbers[8] = {
		1, 17, 9, 14, 11, 6, 5, 1
	};
*/
	bool test_data[4 * 4* 4]  = {
		1, 0, 0, 0,  1, 1, 0, 0,  1, 1, 1, 0,  0, 1, 1, 1,
		1, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,
		1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
		1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0
	};

	C3DSelectBig f;

	C3DUSImage src(C3DBounds(4,4,4), input_data);

	P3DImage result_wrap = f.filter(src);
	const C3DBitImage *result = dynamic_cast<const C3DBitImage *>(result_wrap.get());
	BOOST_REQUIRE(result);

	size_t k = 0;
	for (C3DBitImage::const_iterator i = result->begin(), e = result->end();
	     i != e; ++i,  ++k)
		BOOST_CHECK_EQUAL(*i, test_data[k]);
}
