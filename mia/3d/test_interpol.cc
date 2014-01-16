/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <mia/3d/interpolator.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

CSplineKernelTestPath spline_kernel_init_path; 


template <typename T, typename  I, bool is_int>
struct __dispatch_check {
	static void apply(T x, T y) {
		BOOST_CHECK_EQUAL(x,y); 
	}
	static void apply_xyz(const I& /*src*/, int /*x*/, int /*y*/, int /*z*/) {
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


static void test_external_cache_interpolator() 
{
	T3DDatafield<float> data(C3DBounds(10, 12, 11));
	auto kernel = produce_spline_kernel(bspline3); 
	
	auto i = data.begin();
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i)
				*i = x + y + z + 1;

	T3DConvoluteInterpolator<float>  src(data, kernel);
	
	auto cache = src.create_cache(); 
	i = data.begin();

	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i) {
				C3DFVector loc(x,y,z);
				auto v = src(loc, cache);
				BOOST_CHECK_CLOSE(v, *i, 0.01); 
			}
}


struct FParallelInterpolator {
	T3DDatafield<float>& output; 
	const T3DConvoluteInterpolator<float>& src; 
	
	FParallelInterpolator(T3DDatafield<float>& _output, 
			      const T3DConvoluteInterpolator<float>& _src):
		output(_output), 
		src(_src)
		{
		}

	void operator()( const tbb::blocked_range<int>& range ) const {
		
		auto cache = src.create_cache(); 
		for (auto z = range.begin(); z != range.end(); ++z)
			for (size_t y = 0; y < output.get_size().y; ++y)
				for (size_t x = 0; x < output.get_size().x; ++x) {
					C3DFVector loc(x,y,z);
					output(x,y,z) = src(loc, cache);
				}
		
	}
}; 


struct FParallelInterpolator2 {
	T3DDatafield<float>& output; 
	const T3DConvoluteInterpolator<float>& src; 
	const C3DFVector& shift; 
	const T3DDatafield<float>& test_data; 
	
	FParallelInterpolator2(T3DDatafield<float>& _output, 
			       const T3DConvoluteInterpolator<float>& _src, 
			       const C3DFVector& _shift, 
			       const T3DDatafield<float>& _test_data
			       
		):
		output(_output), 
		src(_src), 
		shift(_shift), 
		test_data(_test_data)
		{
		}

	void operator()( const tbb::blocked_range<int>& range ) const {
		CThreadMsgStream thread_stream;
		auto cache = src.create_cache(); 
		for (auto z = range.begin(); z != range.end(); ++z)
			for (size_t y = 0; y < output.get_size().y; ++y)
				for (size_t x = 0; x < output.get_size().x; ++x) {
					C3DFVector loc(x,y,z);
					output(x,y,z) = src(loc + shift, cache);
					cvmsg() << loc << " weights:" 
						<< " x = " << cache.x.weights << cache.x.index
						<< " y = " << cache.y.weights << cache.y.index 
						<< " z = " << cache.z.weights << cache.z.index
						<< " res= " << output(x,y,z)
						<< " test= " << test_data(x,y,z)
						<< "\n"; 
				}
		
	}
}; 

static void test_parallel_interpolator() 
{
	T3DDatafield<float> data(C3DBounds(10, 12, 11));
	auto kernel = produce_spline_kernel(bspline3); 
	
	auto i = data.begin();
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i)
				*i = x + y + z + 1;

	T3DConvoluteInterpolator<float>  src(data, kernel);
	
	T3DDatafield<float> output(data.get_size());
	FParallelInterpolator worker(output, src); 
	
	tbb::parallel_for(tbb::blocked_range<int>( 0, data.get_size().z), worker);
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x)
				BOOST_CHECK_CLOSE(output(x,y,z), data(x,y,z), 0.01); 
	
}


static void test_parallel_interpolator_zerofill_shifted() 
{
	C3DFVector shift(1,2,3); 

	T3DDatafield<float> data(C3DBounds(9, 8, 7));
	T3DDatafield<float> test_data(C3DBounds(9, 8, 7));
	auto kernel = produce_spline_kernel(bspline0); 
	auto bc = produce_spline_boundary_condition("zero"); 
	
	auto i = data.begin();
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i) {
				*i = x + 10 * y + 100 * z;
				if (x >= 1 && y >= 2 && z >= 3) 
					test_data(x - 1, y - 2, z - 3) = *i; 
			}

	T3DConvoluteInterpolator<float>  src(data, kernel, *bc, *bc, *bc);
	
	T3DDatafield<float> output(data.get_size());
	FParallelInterpolator2 worker(output, src, shift, test_data); 
	
	tbb::parallel_for(tbb::blocked_range<int>( 0, data.get_size().z), worker);
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x) {
				if (output(x,y,z) != test_data(x,y,z)) {
					cvfail() << "FAIL:" << x<< "," << y << "," << z << ": "
						  << output(x,y,z) << " vs "<< test_data(x,y,z) << "\n"; 
				}
				BOOST_CHECK_CLOSE(output(x,y,z), test_data(x,y,z), 0.01); 
			}
	
}



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
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline1>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline1>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline1>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline1>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline2>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline2>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline2>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline2>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline2>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline3>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline3>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline3>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline3>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline4>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline4>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline4>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline4>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline4>)));


	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline5>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline5>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline5>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline5>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline5>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, bspline0>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, bspline0>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, bspline0>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, bspline0>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, bspline0>)));

	suite->add( BOOST_TEST_CASE(( &test_type<unsigned char, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed char, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned short, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<signed short, omomsspl3>)));
#ifdef LONG_64BIT
	suite->add( BOOST_TEST_CASE(( &test_type<signed long, omomsspl3>)));
	suite->add( BOOST_TEST_CASE(( &test_type<unsigned long, omomsspl3>)));
#endif
	suite->add( BOOST_TEST_CASE( (&test_type<float, omomsspl3>)));
	suite->add( BOOST_TEST_CASE( (&test_type<double, omomsspl3>)));

	suite->add( BOOST_TEST_CASE( (&test_external_cache_interpolator))); 
	suite->add( BOOST_TEST_CASE( (&test_parallel_interpolator))); 

	suite->add( BOOST_TEST_CASE( (&test_parallel_interpolator_zerofill_shifted))); 

}
