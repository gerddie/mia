/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
#include <mia/core/type_traits.hh>
#include <mia/3d/filter/convert.hh>
#include <boost/mpl/insert_range.hpp>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;

namespace bmpl=boost::mpl;

template <typename T, typename S, int method>
struct test_data{
	static pair<S, S> get_source() {
		BOOST_STATIC_ASSERT(sizeof(T) == 0);
	}
	static pair<T, T> get_target() {
		BOOST_STATIC_ASSERT(sizeof(T) == 0);
	}
};

template <typename T>
struct test_data<T, T, pc_copy> {
	static pair<T, T> get_source() {
		return get_minmax<T>::apply();
	}
	static pair<T, T> get_target() {
		return get_minmax<T>::apply();
	}
};

template <typename T, bool is_float>
struct __dispatch_get_range {
	static pair<T, T> apply() {
		pair<T, T> result = get_minmax<T>::apply();
		T help = result.first;
		result.first  = result.second / 4 - help / 4 + help;
		result.second = result.second / 2 - help / 2 + help;
		return result;
	}
};

template <typename T>
struct __dispatch_get_range<T, false> {
	static pair<T, T> apply() {
		pair<T, T> result = get_minmax<T>::apply();
		T help = result.first;
		result.first  = __mia_round<T, false>::apply((double(result.second) - double(help)) / 4.0 + help);
		result.second = __mia_round<T, false>::apply((double(result.second) - double(help)) / 2.0 + help);
		return result;
	}
};

template <typename T, typename S>
struct test_data<T, S, pc_range> {
	static pair<S, S> get_source() {
		return __dispatch_get_range<S, std::is_floating_point<S>::value >::apply();
	}
	static pair<T, T> get_target() {
		return __dispatch_get_range<T, std::is_floating_point<T>::value >::apply();
	}
};


template <typename T>
struct test_data<T, bool, pc_range> {
	static pair<bool, bool> get_source() {
		return __dispatch_get_range<bool, false >::apply();
	}
	static pair<T, T> get_target() {
		return get_minmax<T>::apply();
	}
};

template <typename T, typename S>
struct test_data<T, S, pc_opt> {
	static pair<S, S> get_source() {
		pair<S, S> result = get_minmax<S>::apply();
		result.first = result.second / 4;
		result.second /= 2;
		return result;
	}
	static pair<T, T> get_target() {
		return get_minmax<T>::apply();
	}
};

template <typename T>
struct test_data<T, bool, pc_opt> {
	static pair<bool, bool> get_source() {
		return  pair<bool, bool>(false, true);
	}
	static pair<T, T> get_target() {
		return get_minmax<T>::apply();
	}
};

template <typename T, typename S, bool T_is_float>
struct __dispatch_target_copy {
	static pair<T, T>  apply() {
		pair<S, S> source_range = get_minmax<S>::apply();
		pair<T, T> target_range = get_minmax<T>::apply();

		target_range.first = (double)source_range.first > (double)target_range.first ?
			static_cast<T>(source_range.first) : target_range.first;

		target_range.second = (double)source_range.second < (double)target_range.second ?
			static_cast<T>(source_range.second) : target_range.second;
		return target_range;
	}
};

template <typename T, typename S>
struct __dispatch_target_copy<T, S, true> {
	static pair<T, T>  apply() {
		pair<S, S> source_range = get_minmax<S>::apply();
		return pair<T, T>(source_range.first, source_range.second);
	}
};

template <typename T, typename S>
struct test_data<T, S, pc_copy> {
	static pair<S, S> get_source() {
		return get_minmax<S>::apply();
	}
	static pair<T, T> get_target() {
		return __dispatch_target_copy<T, S, std::is_floating_point<T>::value>::apply();
	}
};

template <typename T, typename S, int method>
static void t_run_test()
{
	T3DImage<S> source(C3DBounds(2,1,1));

	pair<S,S> source_copy = test_data<T,S, method>::get_source();
	pair<T,T> target_copy = test_data<T,S, method>::get_target();

	cvdebug() << "values:" << source_copy.first << "," << source_copy.second << "\n";

	source(0,0,0) = source_copy.first;
	source(1,0,0) = source_copy.second;

	C3DImageConvert conv((EPixelType)pixel_type<T>::value, (EPixelConversion)method, 1.0, 0.0);

	P3DImage conv_image = conv.filter(source);
	T3DImage<T> *result = dynamic_cast<T3DImage<T> *>(conv_image.get());

	BOOST_REQUIRE(result);

	cvdebug() << "method:" << method << " " << (double)(*result)(0,0,0) << " expect: " <<  (double)target_copy.first
		  << " && " << (double)(*result)(1,0,0) << " expect: " <<  (double)target_copy.second << '\n';

	BOOST_CHECK_EQUAL((*result)(0,0,0), target_copy.first);
	BOOST_CHECK_EQUAL((*result)(1,0,0), target_copy.second);
}

template <typename T, typename S>
static void do_run_tests()
{
	cvdebug() << ( int )pixel_type<S>::value << "->" << (int)pixel_type<T>::value <<"\n";
	t_run_test<T,S,pc_copy>();
	t_run_test<T,S,pc_range>();
	t_run_test<T,S,pc_opt>();
}


template <typename S, typename T>
struct STestTypes {
	typedef S SourceType;
	typedef T TargetType;
};

template <class T>
struct __PixelTypes{
	typedef bmpl::vector<STestTypes<T, signed char>,
			     STestTypes<T, unsigned char>,
			     STestTypes<T, signed short>,
			     STestTypes<T, unsigned short>,
			     STestTypes<T, signed int>,
			     STestTypes<T, unsigned int>,
			     STestTypes<T, float>,
			     STestTypes<T, double>
#ifdef HAVE_INT64
			     STestTypes<T, mia_int64>,
			     STestTypes<T, mia_uint64>,
#endif
			     > type;
};

#define TEST_TYPE_COMBINATIONS(NAME,TYPE) \
	BOOST_AUTO_TEST_CASE_TEMPLATE( NAME##test_convert, T , __PixelTypes<TYPE>::type ) \
	{ \
		do_run_tests<typename T::SourceType, typename T::TargetType>(); \
	}

TEST_TYPE_COMBINATIONS(sc, signed char)
TEST_TYPE_COMBINATIONS(uc, unsigned char)
TEST_TYPE_COMBINATIONS(ss, signed short)
TEST_TYPE_COMBINATIONS(us,unsigned short)
TEST_TYPE_COMBINATIONS(si, signed int)
TEST_TYPE_COMBINATIONS(ui, unsigned int)
TEST_TYPE_COMBINATIONS(f, float)
TEST_TYPE_COMBINATIONS(d, double)
