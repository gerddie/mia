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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/type_traits.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/core/interpolator1d.hh>

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


typedef bmpl::vector<CBSplineKernel3,
		     CBSplineKernel4
		     ,CBSplineKernel5
		     > test_kernels2;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_derivatives2, T, test_kernels2)
{
	const double x = 0.2;
	T kernel;
	vector<double> weights(kernel.size());
	vector<double> pweights(kernel.size());
	vector<double> mweights(kernel.size());
	vector<int> index(kernel.size()); 
		
	kernel.derivative(x, weights, index, 2); 
	kernel.get_derivative_weights(x + 0.00005, pweights);
	kernel.get_derivative_weights(x - 0.00005, mweights);

	for (size_t i = 0; i < weights.size(); ++i) {
		double test = (pweights[i] - mweights[i]) / 0.0001;
		BOOST_CHECK_CLOSE(weights[i], test, 0.01);
	}
	
}



BOOST_AUTO_TEST_CASE(  test_spline2_weight_at )
{
	CBSplineKernel2 kernel; 
	
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2, 0) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1, 0), 0.125, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0, 0), 0.75, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1, 0), 0.125, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2, 0) + 1.0, 1.0, 0.1);  

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2, 1) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1, 1)      , 0.5, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0, 1) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1, 1)      ,-0.5, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2, 1) + 1.0, 1.0, 0.1);  

	BOOST_CHECK_THROW(kernel.get_weight_at( 2, 2), invalid_argument); 
}


BOOST_AUTO_TEST_CASE(  test_bspline3_weight_at )
{
	CBSplineKernel3 kernel; 
	
	std::vector<double> weight(4); 
	std::vector<int> index(4); 
	kernel(-.5, weight, index); 

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.5, 0) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.5, 0) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.5, 0), weight[0], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.5, 0), weight[1], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.5, 0), weight[2], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.5, 0), weight[3], 0.1);  

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.5, 1) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.5, 1) + 1.0, 1.0, 0.1);  
	
	kernel.derivative(-0.5, weight, index); 
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.5, 1), weight[3], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.5, 1), weight[2], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.5, 1), weight[1], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.5, 1), weight[0], 0.1);  
	

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.5, 2) + 1.0, 1.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.5, 2) + 1.0, 1.0, 0.1);  
	
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.5, 2), 0.5, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.5, 2), -0.5, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(   0, 2), -2.0, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.5, 2), -0.5, 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.5, 2), 0.5, 0.1);  

	BOOST_CHECK_THROW(kernel.get_weight_at( 2, 3), invalid_argument); 
}



BOOST_AUTO_TEST_CASE(  test_bspline3_weight_at_b )
{
	CBSplineKernel3 kernel; 
	
	std::vector<double> weight(4); 
	std::vector<int> index(4); 
	kernel(1.75, weight, index); 

	BOOST_CHECK_CLOSE(kernel.get_weight_at( -1.25, 0), weight[3], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.25, 0), weight[2], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.75, 0), weight[1], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.75, 0), weight[0], 0.1);  


	kernel.derivative(1.75, weight, index); 
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.25, 1), weight[3], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.25, 1), weight[2], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.75, 1), weight[1], 0.1);  
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.75, 1), weight[0], 0.1);  

	
}



BOOST_AUTO_TEST_CASE(  test_bspline2_derivatives )
{
	CBSplineKernel2 kernel; 
	vector<double> weight(kernel.size()); 
	vector<int>    index(kernel.size()); 
	kernel.derivative(1.0, weight, index, 0); 
	BOOST_CHECK_CLOSE(weight[0], 0.125, 0.1); 
	BOOST_CHECK_CLOSE(weight[1], 0.75, 0.1); 
	BOOST_CHECK_CLOSE(weight[2], 0.125, 0.1); 

	kernel.derivative(1.0, weight, index, 1); 
	BOOST_CHECK_CLOSE(weight[0],  -.5, 0.1); 
	BOOST_CHECK_CLOSE(weight[1], 0.0, 0.1); 
	BOOST_CHECK_CLOSE(weight[2],   .5, 0.1); 

	kernel.derivative(1.0, weight, index, 2); 
	BOOST_CHECK_CLOSE(weight[0],   1, 0.1); 
	BOOST_CHECK_CLOSE(weight[1],  -2, 0.1); 
	BOOST_CHECK_CLOSE(weight[2],   1, 0.1); 
	
}


BOOST_AUTO_TEST_CASE(  test_bspline3_derivatives )
{
	CBSplineKernel3 kernel; 
	vector<double> weight(kernel.size()); 
	vector<int>    index(kernel.size()); 
	kernel.derivative(0.5, weight, index, 0); 
	BOOST_CHECK_CLOSE(weight[3], 0.020833, 0.1); 
	BOOST_CHECK_CLOSE(weight[2], 0.47917, 0.1); 
	BOOST_CHECK_CLOSE(weight[1], 0.47917, 0.1); 
	BOOST_CHECK_CLOSE(weight[0], 0.020833, 0.1);
	BOOST_CHECK_EQUAL(index[0], -1);  
	BOOST_CHECK_EQUAL(index[1], 0); 
	BOOST_CHECK_EQUAL(index[2], 1); 
	BOOST_CHECK_EQUAL(index[3], 2); 

	kernel.derivative(0.5, weight, index, 1); 
	BOOST_CHECK_CLOSE(weight[3],   .125, 0.1); 
	BOOST_CHECK_CLOSE(weight[2],  0.625, 0.1); 
	BOOST_CHECK_CLOSE(weight[1], -0.625, 0.1); 
	BOOST_CHECK_CLOSE(weight[0], -0.125, 0.1); 
	
	kernel.derivative(0.5, weight, index, 2); 
	BOOST_CHECK_CLOSE(weight[3],    0.5, 0.1); 
	BOOST_CHECK_CLOSE(weight[2],   -0.5, 0.1); 
	BOOST_CHECK_CLOSE(weight[1],   -0.5, 0.1); 
	BOOST_CHECK_CLOSE(weight[0],    0.5, 0.1); 
	
}


