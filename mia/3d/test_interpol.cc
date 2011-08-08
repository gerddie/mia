/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/3d/interpolator.hh>
#include <mia/core/msgstream.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

CSplineKernelTestPath spline_kernel_init_path; 


template <typename T, typename  I, bool is_int>
struct __dispatch_check {
	static void apply(T x, T y) {
		BOOST_CHECK_EQUAL(x,y); 
	}
	static void apply_xyz(const I& src, int x, int y, int z) {
/*
		C3DFVector locx(x + 0.4,y,z);
		const T value  = T(rint(x + y + z + 0.4)); 
		BOOST_CHECK_EQUAL(src(locx), value); 
		
		C3DFVector locy(x,y + 0.4,z);
		BOOST_CHECK_EQUAL(src(locy), value); 
		
		C3DFVector locz(x,y,z + 0.4);
		BOOST_CHECK_EQUAL(src(locz), value); 
*/
	}
}; 

template <typename T, typename I>
struct __dispatch_check<T, I, false> {
	static void apply(T x, T y) {
		BOOST_CHECK_CLOSE(x,y, 0.01); 
	}
	static void apply_xyz(const I& src, int x, int y, int z) {
		C3DFVector locx(x + 0.5,y,z);
		const double value  = x + y + z + 1.5; 
		BOOST_CHECK_CLOSE(src(locx), value, 2); 
		
		C3DFVector locy(x,y + 0.5,z);
		BOOST_CHECK_CLOSE(src(locy), value, 2); 
		
		C3DFVector locz(x,y,z + 0.5);
		BOOST_CHECK_CLOSE(src(locz), value, 2); 
	}
}; 

template <class Data3D, class Interpolator>
void test_interpolator(const Data3D& data, const Interpolator& src)
{
	typedef typename Data3D::value_type T; 
	const bool is_int = std::is_integral<T>::value; 

	auto i = data.begin();

	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i) {
				C3DFVector loc(x,y,z);
				auto v = src(loc);
				__dispatch_check<T, Interpolator, is_int>::apply(v, *i); 
			}
}


template <class Data3D, class Interpolator>
void test_interpolator_zerofive(const Data3D& data, const Interpolator& src)
{
	typedef typename Data3D::value_type T; 
	const bool is_int = std::is_integral<T>::value; 

	for (size_t z = 0; z < data.get_size().z - 1; ++z)
		for (size_t y = 0; y < data.get_size().y - 1; ++y)
			for (size_t x = 0; x < data.get_size().x - 1; ++x) {
				__dispatch_check<T, Interpolator, is_int>::apply_xyz(src, x, y, z); 
			}
}


template <class T, template <class> class Interpolator>
static void  test_direct_interpolator(const T3DDatafield<T>& data)
{
	Interpolator<T> src(data);
	test_interpolator(data, src);
}


template <class T>
static void test_conv_interpolator(const T3DDatafield<T>& data, PSplineKernel kernel)
{
	T3DConvoluteInterpolator<T>  src(data, kernel);
	test_interpolator(data, src);
	if (kernel->size() == 2) 
		test_interpolator_zerofive(data, src);
}

template <class T, const char *kernel>
static void test_type()
{
	T3DDatafield<T> data(C3DBounds(10, 12, 11));

	typename T3DDatafield<T>::iterator i = data.begin();
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i)
				*i = x + y + z + 1;
	test_conv_interpolator<T>(data, produce_spline_kernel(kernel)); 
}

extern const char bspline0[] = "bspline:d=0"; 
extern const char bspline1[] = "bspline:d=1"; 
extern const char bspline2[] = "bspline:d=2"; 
extern const char bspline3[] = "bspline:d=3"; 
extern const char bspline4[] = "bspline:d=4"; 
extern const char bspline5[] = "bspline:d=5"; 
extern const char omomsspl3[] = "omoms:d=3"; 

static double omoms3(double x)
{
	if (x < 0)
		x = -x;

	if (x >= 2)
		return 0;
	if (x >= 1)
		return ((1 - 1.0 / 6.0 * x) * x - 85.0/42.0 ) * x + 29.0 / 21.0;

	return ((0.5 * x - 1) * x + 1/14.0) * x + 13.0 / 21.0;
}

static void test_omoms3()
{
	const double x = 0.2;
	auto kernel = produce_spline_kernel("omoms:d=3"); 
	std::vector<double> weights(kernel->size());
	kernel->get_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		if (fabs(weights[3 - i] - omoms3( x - 2.0 + i)) > 1e-4)
			cvfail() << "omoms coeff FAILED" << std::endl;
	}
}

void add_3dinterpol_tests( boost::unit_test::test_suite* suite)
{
	suite->add( BOOST_TEST_CASE( &test_omoms3));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline1>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline1>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline1>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline1>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline1>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline1>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline1>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline1>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline2>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline2>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline2>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline2>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline3>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline3>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline3>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline3>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline4>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline4>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline4>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline4>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline5>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline5>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline5>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline5>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline0>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, bspline0>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline0>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline0>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, omomsspl3>)));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE(( &test_type<mia_int64, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<mia_uint64, omomsspl3>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, omomsspl3>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, omomsspl3>)));




}
