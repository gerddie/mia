/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <climits>

#define BOOST_TEST_DYN_LINK

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/type_traits.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/core/interpolator1d.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>


using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;
namespace bmpl=boost::mpl;

NS_MIA_USE
using namespace std;
using namespace boost;


template <typename T, bool is_float>
struct __dispatch_compare {
	static void apply(T a, T b) {
		BOOST_CHECK_EQUAL( a, b);
	}
};

template <typename T>
struct __dispatch_compare<T, true> {
	static void apply(T a, T b) {
		BOOST_CHECK_CLOSE(1.0 + a, 1.0 + b, 0.01);
	}
};


template <class T, class Interpolator>
void test_interpolator(const vector<T>& data, const Interpolator& src)
{

	typename vector<T>::const_iterator i = data.begin();

	for (size_t x = 0; x < data.size(); ++x, ++i) {
		__dispatch_compare<T, boost::is_floating_point<T>::value >::apply( src(x), data[x]);
	}
}


template <class T, template <class> class Interpolator>
void test_direct_interpolator(const vector<T>& data)
{
	Interpolator<T> src(data);
	test_interpolator(data, src);
}


template <class T>
void test_conv_interpolator(const vector<T>& data, SHARED_PTR(CBSplineKernel) kernel)
{
	T1DConvoluteInterpolator<T>  src(data, kernel);
	test_interpolator(data, src);
}


template <class T>
void test_type()
{
	vector<T> data(10);


	typename vector<T>::iterator i = data.begin();

	for (size_t x = 1; x < data.size() + 1; ++x, ++i)
		*i = T(x);

	test_direct_interpolator<T, T1DNNInterpolator>(data);
	test_direct_interpolator<T, T1DLinearInterpolator>(data);
	test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel)(new CBSplineKernel2()));
	test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel)(new CBSplineKernel3()));
	test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel)(new CBSplineKernel4()));
	test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel)(new CBSplineKernel5()));
	test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel)(new CBSplineKernelOMoms3()));
}

double omoms3(double x)
{
	if (x < 0)
		x = -x;

	if (x >= 2)
		return 0;
	if (x >= 1)
		return ((1 - 1.0 / 6.0 * x) * x - 85.0/42.0 ) * x + 29.0 / 21.0;

	return ((0.5 * x - 1) * x + 1/14.0) * x + 13.0 / 21.0;
}

BOOST_AUTO_TEST_CASE(  test_omoms3 )
{
	const double x = 0.2;
	CBSplineKernelOMoms3 kernel;
	std::vector<double> weights(kernel.size());
	kernel.get_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		BOOST_CHECK_CLOSE(weights[3 - i], omoms3( x - 2.0 + i), 1e-4);
	}
}

BOOST_AUTO_TEST_CASE(  test_omoms3_derivative )
{
	const double x = 0.2;
	CBSplineKernelOMoms3 kernel;
	std::vector<double> weights(kernel.size());
	kernel.get_derivative_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		BOOST_CHECK_CLOSE(weights[3 - i], (omoms3( x - 2.0 + i + 0.0005) -
						   omoms3( x - 2.0 + i - 0.0005)) / 0.001 ,
				  0.01);
	}
}

BOOST_AUTO_TEST_CASE( test_types )
{
	test_type<unsigned char>();
	test_type<signed char>();
	test_type<unsigned short>();
	test_type<signed short>();
#ifdef HAVE_INT64
	test_type<mia_uint64>();
	test_type<mia_int64>();
#endif
	test_type<float>();
	test_type<double>();

}

typedef bmpl::vector<CBSplineKernel2,
		     CBSplineKernel3,
		     CBSplineKernel4,
		     CBSplineKernel5> test_kernels;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_derivatives, T, test_kernels)
{
	const double x = 0.2;
	T kernel;
	std::vector<double> weights(kernel.size());
	std::vector<double> pweights(kernel.size());
	std::vector<double> mweights(kernel.size());
	kernel.get_derivative_weights(x, weights);
	kernel.get_weights(x + 0.00005, pweights);
	kernel.get_weights(x - 0.00005, mweights);

	for (size_t i = 0; i < weights.size(); ++i) {
		double test = (pweights[i] - mweights[i]) / 0.0001;
		BOOST_CHECK_CLOSE(weights[i], test, 0.01);
	}
}
