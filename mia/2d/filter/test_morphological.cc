/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
#include <mia/2d/filter/morphological.hh>

namespace bmpl=boost::mpl;

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace morphological_2dimage_filter;

struct CTestShape: public C2DShape {
	CTestShape() {
		insert(C2DShape::Flat::value_type( 0, 0));
		insert(C2DShape::Flat::value_type( 1, 0));
		insert(C2DShape::Flat::value_type( 0, 1));
	}

};

template <typename T>
struct MorphologicalFixture {
	MorphologicalFixture(C2DBounds s, T *init_data);

	void check(const C2DFilter& f,  T *test_data);

	P2DShape shape;
	C2DBounds size;
	T2DImage<T> src;
};

template <typename T>
MorphologicalFixture<T>::MorphologicalFixture(C2DBounds s, T *init_data):
	shape(new CTestShape()),
	size(s),
	src(size)
{
	copy(init_data, init_data + src.size(), src.begin());
}

template <typename T>
void MorphologicalFixture<T>::check(const C2DFilter& f,  T *test_data)
{
	P2DImage result = f.filter(src);

	const T2DImage<T> * presult = dynamic_cast<const T2DImage<T> *>(result.get());
	BOOST_REQUIRE(presult);

	BOOST_CHECK_EQUAL(presult->get_size(), size);
	BOOST_REQUIRE(presult->get_size() == size);

	for (typename T2DImage<T>::const_iterator r= presult->begin();
	     r != presult->end();  ++r, ++test_data)
		BOOST_CHECK_EQUAL(*r,*test_data);
}

typedef bmpl::vector<signed char,
		     unsigned char,
		     signed short,
		     unsigned short,
		     signed int,
		     unsigned int,
		     float,
		     double
#ifdef HAVE_INT64
		     mia_int64,
		     mia_uint64,
#endif
		     > type_list;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_erode, T, type_list)
{
	C2DBounds size(2,2);
	T init_values[4] = {5,4,
			    1,2};
	T test_values[4] = {1,2,
			    1,2};

	MorphologicalFixture<T> fixture(size, init_values);
	fixture.check(C2DErode(fixture.shape, true), test_values);
	fixture.check(C2DErode(fixture.shape, false), test_values);
}

BOOST_AUTO_TEST_CASE(test_erode_bit)
{
	C2DBounds size(2,2);
	bool init_values[4] = { true, false,
				true, true };

	bool test_values[4] = {false, false,
			       true, true };

	MorphologicalFixture<bool> fixture(size, init_values);
	fixture.check(C2DErode(fixture.shape, true), test_values);
	fixture.check(C2DErode(fixture.shape, false), test_values);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(test_dilate, T, type_list)
{
	C2DBounds size(2,2);
	T init_values[4] = {1,3,
			    4,5};
	T test_values[4] = {4,5,
			    5,5};

	MorphologicalFixture<T> fixture(size, init_values);
	fixture.check(C2DDilate(fixture.shape, true), test_values);
	fixture.check(C2DDilate(fixture.shape, false), test_values);
}

BOOST_AUTO_TEST_CASE(test_dilate_bit)
{
	C2DBounds size(2,2);
	bool init_values[4] = { false, false,
				true ,false };

	bool test_values[4] = { true, false,
			       true, false };

	MorphologicalFixture<bool> fixture(size, init_values);
	fixture.check(C2DDilate(fixture.shape, true), test_values);
	fixture.check(C2DDilate(fixture.shape, false), test_values);
}



BOOST_AUTO_TEST_CASE_TEMPLATE(test_open, T, type_list)
{
	C2DBounds size(3,3);
	T init_values[9] = {
		1, 3, 3,
		4, 5, 3,
		5, 2, 1
	};
	T test_values[9] = {
		4, 3, 3,
		4, 2, 1,
		2, 1, 1
	};

	MorphologicalFixture<T> fixture(size, init_values);
	fixture.check(C2DOpenClose(fixture.shape, true, true), test_values);
	fixture.check(C2DOpenClose(fixture.shape, false, true), test_values);
}

BOOST_AUTO_TEST_CASE(test_open_bit)
{
	C2DBounds size(3,3);
	bool init_values[9] = {
		false, true, false,
		true , true, true,
		true , false,true
	};

	bool test_values[9] = {
		true,  false, true,
		true , true, true,
		false, true, true
	};

	MorphologicalFixture<bool> fixture(size, init_values);
	fixture.check(C2DOpenClose(fixture.shape, true, true), test_values);
	fixture.check(C2DOpenClose(fixture.shape, false, true), test_values);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_close, T, type_list)
{
	C2DBounds size(3,3);
	T init_values[9] = {
		1, 3, 3,
		4, 5, 3,
		5, 2, 1
	};
	T test_values[9] = {
		4, 3, 3,
		5, 2, 1,
		2, 1, 1
	};

	MorphologicalFixture<T> fixture(size, init_values);
	fixture.check(C2DOpenClose(fixture.shape, true, false), test_values);
	fixture.check(C2DOpenClose(fixture.shape, false, false), test_values);
}

BOOST_AUTO_TEST_CASE(test_close_bit)
{
	C2DBounds size(3,3);
	bool init_values[9] = {
		false, true, false,
		false, false,false,
		true , false,true
	};

	bool test_values[9] = {
		true,  false,  false,
		false,  false, true,
		true,  true, true
	};

	MorphologicalFixture<bool> fixture(size, init_values);
	fixture.check(C2DOpenClose(fixture.shape, true, false), test_values);
	fixture.check(C2DOpenClose(fixture.shape, false, false), test_values);
}


template <typename P>
void test_create_templ(const string& init)
{
	auto m = BOOST_TEST_create_from_plugin<P>(init.c_str()); 
	BOOST_CHECK_EQUAL(m->get_init_string(), init); 
}


BOOST_AUTO_TEST_CASE(test_creation)
{
	test_create_templ<C2DErodeFilterFactory>("erode:shape=4n,hint=white"); 
	test_create_templ<C2DDilateFilterFactory>("dilate:shape=4n,hint=black"); 
	test_create_templ<C2DOpenFilterFactory>("open:shape=8n,hint=white");
	test_create_templ<C2DCloseFilterFactory>("close:shape=4n,hint=white"); 
}

