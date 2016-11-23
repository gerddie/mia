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

#define BOOST_TEST_DYN_LINK

#include <stdexcept>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/mpl/vector.hpp>

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
	TestPixelType():TestPixelTypeBase((EPixelType)pixel_type<T>::value), 
			value(T())
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
	do_test_combiner_call_b<T, int8_t>(dual);
	do_test_combiner_call_b<T, int16_t>(dual);
	do_test_combiner_call_b<T, int32_t>(dual);
	do_test_combiner_call_b<T, int64_t>(dual);
	do_test_combiner_call_b<T, uint8_t>(dual);
	do_test_combiner_call_b<T, uint16_t>(dual);
	do_test_combiner_call_b<T, uint32_t>(dual);
	do_test_combiner_call_b<T, uint64_t>(dual);
	do_test_combiner_call_b<T, float>(dual);
	do_test_combiner_call_b<T, double>(dual);
}

NS_MIA_END

NS_MIA_USE

typedef boost::mpl::vector<bool,
		     int8_t,
		     uint8_t,
		     int16_t,
		     uint16_t,
		     int32_t,
		     uint32_t,
		     int64_t,
		     uint64_t,
		     float,
		     double
		     > test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE( test_filter_call, T, test_types)
{
	UniFilter uni;
	do_test_filter_call<T>(uni);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_combiner_call, T, test_types)
{
	DualFilter dual;
	do_test_combiner_call<T>(dual);
}
