/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#include <mia/core/type_traits.hh>
#include <boost/test/unit_test.hpp>
//#include <boost/test/floating_point_comparison.hpp>

#include <mia/2d/imagetest.hh>

NS_MIA_BEGIN
using namespace boost;

template <typename T, typename R>
struct __compare {
	static void apply(const T2DImage<T>& /*a*/, const T2DImage<R>& /*b*/) {
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
		BOOST_CHECK_CLOSE(test, expect, 0.1);
	}
};

template <typename T>
struct __compare<T,T> {
	static void apply(const T2DImage<T>& a, const T2DImage<T>& b) {
		const bool is_fp = is_floating_point<T>::value;
		typename T2DImage<T>::const_iterator ia = a.begin();
		typename T2DImage<T>::const_iterator ie = a.end();
		typename T2DImage<T>::const_iterator ib = b.begin();


		while (ia != ie) {
			__check_equal<T, is_fp>::apply(*ia, *ib);
			++ia;
			++ib;
		}
	}
};

class C2DImageCompare: public TFilter<bool>  {
public:
	template <typename  T, typename  S>
	C2DImageCompare::result_type operator()(const T2DImage<T>& a, const T2DImage<S>& b) const
	{
		__compare<T, S>::apply(a,b);
		return C2DImageCompare::result_type();
	}
};

void  EXPORT_2D test_image_equal(const C2DImage& test, const C2DImage& expect)
{
	BOOST_CHECK_EQUAL(test.get_size(), expect.get_size());
	BOOST_REQUIRE(test.get_size() == expect.get_size());

	BOOST_CHECK_EQUAL(test.get_pixel_type(), expect.get_pixel_type());

	C2DImageCompare c;
	mia::filter(c, test, expect);

	PAttributeMap attr_test = test.get_attribute_list();
	PAttributeMap attr_expect = expect.get_attribute_list();

	BOOST_CHECK_EQUAL(attr_test->size(), attr_expect->size());

	for (CAttributeMap::const_iterator amap = attr_test->begin();
	     amap != attr_test->end(); ++amap) {
		CAttributeMap::const_iterator bmap = attr_expect->find(amap->first);
		BOOST_CHECK(amap != attr_expect->end());
		if (amap != attr_expect->end())
			BOOST_CHECK_EQUAL(*amap->second, *bmap->second);
	}
}

NS_MIA_END
