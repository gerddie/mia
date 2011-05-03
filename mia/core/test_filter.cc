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

#define BOOST_TEST_DYN_LINK

#include <stdexcept>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/filter.hh>


NS_MIA_BEGIN

struct TestPixelTypeBase {
	TestPixelTypeBase(EPixelType pixel_type):
		m_pixel_type(pixel_type)
	{
	}
	virtual ~TestPixelTypeBase()
	{
	}

	EPixelType get_pixel_type() const
	{
		return m_pixel_type;
	}
private:
	EPixelType m_pixel_type;
};

template <class T>
struct TestPixelType : public TestPixelTypeBase {
	TestPixelType():TestPixelTypeBase((EPixelType)pixel_type<T>::value)
	{
	}

	T value;
};

template <>
struct Binder<TestPixelTypeBase> {
	typedef __bind_all<TestPixelType> Derived;
};


struct  UniFilter : public TFilter<bool> {
	template <typename T>
	bool operator () ( const TestPixelType<T>& /*x*/)const
	{
		return true;
	}
};

template <typename T>
void do_test_filter_call(UniFilter uni)
{
	TestPixelType<T> a;
	TestPixelTypeBase& ax = a;
	BOOST_CHECK(::mia::filter(uni, ax));
}

struct  DualFilter : public TFilter<bool> {
	template <typename T, typename S>
	bool operator () ( const TestPixelType<T>& /*x*/, const TestPixelType<S>& /*y*/)const
	{
		return true;
	}
};

template <typename T, typename S>
void do_test_combiner_call_b(DualFilter dual)
{
	TestPixelType<T> a;
	TestPixelType<S> b;
	TestPixelTypeBase& ax = a;
	TestPixelTypeBase& bx = b;

	BOOST_CHECK(::mia::filter(dual, ax, bx));
}


template <typename T>
void do_test_combiner_call(DualFilter dual)
{
	do_test_combiner_call_b<T, bool>(dual);
	do_test_combiner_call_b<T, signed char>(dual);
	do_test_combiner_call_b<T, unsigned char>(dual);
	do_test_combiner_call_b<T, signed short>(dual);
	do_test_combiner_call_b<T, unsigned short>(dual);
	do_test_combiner_call_b<T, signed int>(dual);
	do_test_combiner_call_b<T, unsigned int>(dual);
#ifdef HAVE_INT64
	do_test_combiner_call_b<T, mia_int64>(dual);
	do_test_combiner_call_b<T, mia_uint64>(dual);
#endif
	do_test_combiner_call_b<T, float>(dual);
	do_test_combiner_call_b<T, double>(dual);
}

NS_MIA_END

NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_filter_call)
{
	UniFilter uni;

	do_test_filter_call<bool>(uni);
	do_test_filter_call<signed char>(uni);
	do_test_filter_call<unsigned char>(uni);
	do_test_filter_call<signed short>(uni);
	do_test_filter_call<unsigned short>(uni);
	do_test_filter_call<signed int>(uni);
	do_test_filter_call<unsigned int>(uni);
#ifdef HAVE_INT64
	do_test_filter_call<mia_int64>(uni);
	do_test_filter_call<mia_uint64>(uni);
#endif
	do_test_filter_call<float>(uni);
	do_test_filter_call<double>(uni);
}

BOOST_AUTO_TEST_CASE( test_combiner_call )
{
	DualFilter dual;

	do_test_combiner_call<bool>(dual);
	do_test_combiner_call<signed char>(dual);
	do_test_combiner_call<unsigned char>(dual);
	do_test_combiner_call<signed short>(dual);
	do_test_combiner_call<unsigned short>(dual);
	do_test_combiner_call<signed int>(dual);
	do_test_combiner_call<unsigned int>(dual);
#ifdef HAVE_INT64
	do_test_combiner_call<mia_int64>(dual);
	do_test_combiner_call<mia_uint64>(dual);
#endif
	do_test_combiner_call<float>(dual);
	do_test_combiner_call<double>(dual);
}
