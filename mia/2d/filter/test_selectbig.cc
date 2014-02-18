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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/selectbig.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace selectbig_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_selectbig )
{
	const unsigned short input_data[4*4] = {
		1, 2, 2, 3,
		1, 1, 3, 3,
		1, 1, 3, 3,
		1, 6, 6, 6,
	};

	bool test_data[4 * 4]  = {
		1, 0, 0, 0,
		1, 1, 0, 0,
		1, 1, 0, 0,
		1, 0, 0, 0,
	};

	auto  f = BOOST_TEST_create_from_plugin<C2DSelectBigImageFilterFactory>("selectbig"); 
	
	C2DUSImage src(C2DBounds(4,4), input_data);

	P2DImage result_wrap = f->filter(src);
	const C2DBitImage *result = dynamic_cast<const C2DBitImage *>(result_wrap.get());
	BOOST_REQUIRE(result);

	size_t k = 0;
	for (C2DBitImage::const_iterator i = result->begin(), e = result->end();
	     i != e; ++i,  ++k)
		BOOST_CHECK_EQUAL(*i, test_data[k]);
}
