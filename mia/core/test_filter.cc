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
	do_test_combiner_call_b<T, signed char>(dual);
	do_test_combiner_call_b<T, unsigned char>(dual);
	do_test_combiner_call_b<T, signed short>(dual);
	do_test_combiner_call_b<T, unsigned short>(dual);
	do_test_combiner_call_b<T, signed int>(dual);
	do_test_combiner_call_b<T, unsigned int>(dual);
#ifdef LONG_64BIT
	do_test_combiner_call_b<T, unsigned long>(dual);
	do_test_combiner_call_b<T, signed long>(dual);
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
#ifdef LONG_64BIT
	do_test_filter_call<signed long>(uni);
	do_test_filter_call<unsigned long>(uni);
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
#ifdef LONG_64BIT
	do_test_combiner_call<signed long>(dual);
	do_test_combiner_call<unsigned long>(dual);
#endif
	do_test_combiner_call<float>(dual);
	do_test_combiner_call<double>(dual);
}
