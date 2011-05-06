/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Evolutionary Anthropoloy
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


#include <mia/core/shared_ptr.hh>
#include <mia/internal/autotest.hh>
#include <mia/3d/filter/binarize.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;


BOOST_AUTO_TEST_CASE( test_binarize_float )
{
	const float minimum = -1.0;
	const float maximum = 4.0;

	const float input[6] = { 2.0, -1.0, 4.0, 5.0, -1.2, 10.0};
	const bool output[6] = { true, true, true, false, false, false};

	C3DImageBinarize f(minimum, maximum);

	C3DFImage *src = new C3DFImage(C3DBounds(2,3,1), input);
	P3DImage src_wrap(src);

	P3DImage result_wrap = f.filter(*src_wrap);
	const C3DBitImage *result = dynamic_cast<const C3DBitImage *>(result_wrap.get());
	BOOST_REQUIRE(result);

	size_t k = 0;
	for (C3DBitImage::const_iterator i = result->begin(), e = result->end();
	     i != e; ++i,  ++k)
		BOOST_CHECK_EQUAL(*i, output[k]);
}


BOOST_AUTO_TEST_CASE( test_binarize_uint )
{
	const unsigned int minimum = 1;
	const unsigned int  maximum = 4;

	const unsigned int input[6] = { 2, 1, 4, 5, 3, 10};
	const bool output[6] = { true, true, true, false, true, false};

	C3DImageBinarize f(minimum, maximum);

	C3DUIImage *src = new C3DUIImage(C3DBounds(2,3,1), input);
	P3DImage src_wrap(src);

	P3DImage result_wrap = f.filter(*src_wrap);
	const C3DBitImage *result = dynamic_cast<const C3DBitImage *>(result_wrap.get());
	BOOST_REQUIRE(result);

	size_t k = 0;
	for (C3DBitImage::const_iterator i = result->begin(), e = result->end();
	     i != e; ++i,  ++k)
		BOOST_CHECK_EQUAL(*i,  output[k]);
}

