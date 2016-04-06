/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/type_traits.hh>
#include <boost/test/unit_test.hpp>
#include <mia/3d/imagetest.hh>

NS_MIA_BEGIN

template <typename T, typename R>
struct __compare {
	static void apply(const T3DImage<T>& /*a*/, const T3DImage<R>& /*b*/) {
		BOOST_FAIL("Image pixels have different type");
	}
};

template <typename T, bool is_fp>
struct __check_equal{
	static void apply(T test, T expect) {
		BOOST_CHECK_EQUAL(test, expect);
	}
};

template <typename T>
struct __check_equal<T, true>{
	static void apply(T test, T expect) {
		if (expect == 0.0) 
			BOOST_CHECK_SMALL(test, static_cast<T>(1e-6));
		else 
			BOOST_CHECK_CLOSE(test, expect, 0.1);
	}
};

template <typename T>
struct __compare<T,T> {
	static void apply(const T3DImage<T>& a, const T3DImage<T>& b) {
		const bool is_fp = std::is_floating_point<T>::value;
		typename T3DImage<T>::const_iterator ia = a.begin();
		typename T3DImage<T>::const_iterator ie = a.end();
		typename T3DImage<T>::const_iterator ib = b.begin();


		while (ia != ie) {
			__check_equal<T, is_fp>::apply(*ia, *ib);
			++ia;
			++ib;
		}
	}
};

class C3DImageCompare: public TFilter<bool>  {
public:
	template <typename  T, typename  S>
	C3DImageCompare::result_type operator()(const T3DImage<T>& a, const T3DImage<S>& b) const
	{
		__compare<T, S>::apply(a,b);
		return C3DImageCompare::result_type();
	}
};

void  EXPORT_3D test_image_equal(const C3DImage& test, const C3DImage& expect)
{
	BOOST_CHECK_EQUAL(test.get_size(), expect.get_size());
	BOOST_REQUIRE(test.get_size() == expect.get_size());

	BOOST_CHECK_EQUAL(test.get_pixel_type(), expect.get_pixel_type());

	C3DImageCompare c;
	mia::filter(c, test, expect);

	const CAttributedData& attr_test = test;
	const CAttributedData& attr_expect = expect;

	BOOST_CHECK(attr_test == attr_expect);
}

NS_MIA_END
