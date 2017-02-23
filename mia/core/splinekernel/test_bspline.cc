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

#include <mia/internal/autotest.hh>
#include <boost/type_traits.hpp>
#include <mia/core/splinekernel/bspline.hh>
#include <mia/core/interpolator1d.hh>
#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>


using namespace mia; 
using namespace mia::bsplinekernel;
using std::vector; 
using std::invalid_argument; 

namespace bmpl=boost::mpl;

template <typename T, bool is_float>
struct __dispatch_compare {
	static void apply(T a, T b) {
		BOOST_CHECK_EQUAL( a, b);
	}
};

template <typename T>
struct __dispatch_compare<T, true> {
	static void apply(T a, T b) {
		BOOST_CHECK_CLOSE(1.0 + a, 1.0 + b, 0.001);
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

template <class T>
void test_conv_interpolator(const vector<T>& data, PSplineKernel kernel, EInterpolation type, const CSplineBoundaryCondition& bc)
{
	cvdebug() << "test type " << type << ":" << typeid(T).name()<<"\n"; 
	BOOST_CHECK_EQUAL(kernel->get_type(), type); 
	T1DConvoluteInterpolator<T>  src(data, kernel, bc);
	test_interpolator(data, src);
}

typedef bmpl::vector<unsigned char, 
		     signed char, 
		     unsigned short, 
		     signed short, 
#ifdef HAVE_INT64
		     mia_uint64, 
		     mia_int64, 
#endif
		     float, 
		     double> test_pixeltypes;



BOOST_AUTO_TEST_CASE_TEMPLATE(test_type_mirror, T, test_pixeltypes)
{
	vector<T> data(10);


	auto i = data.begin();

	for (size_t x = 1; x < data.size() + 1; ++x, ++i)
		*i = T(x);

	auto bc = produce_spline_boundary_condition("mirror"); ; 
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel0()), ip_bspline0, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel1()), ip_bspline1, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel2()), ip_bspline2, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel3()), ip_bspline3, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel4()), ip_bspline4, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel5()), ip_bspline5, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernelOMoms3()), ip_omoms3, *bc);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(test_type_repeat, T, test_pixeltypes)
{
	vector<T> data = {3,4,8,10,2,3,9,6,7,11,8};



	auto bc = produce_spline_boundary_condition("repeat"); ; 
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel0()), ip_bspline0, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel1()), ip_bspline1, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel2()), ip_bspline2, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel3()), ip_bspline3, *bc);
	test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernelOMoms3()), ip_omoms3, *bc);

	BOOST_CHECK_THROW(test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel4()), ip_bspline4, *bc), 
			  invalid_argument);
	BOOST_CHECK_THROW(test_conv_interpolator<T>(data, PSplineKernel(new CBSplineKernel5()), ip_bspline5, *bc), 
			  invalid_argument);

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
	CSplineKernel::VWeight weights(kernel.size());
	kernel.get_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		BOOST_CHECK_CLOSE(weights[3 - i], omoms3( x - 2.0 + i), 1e-4);
	}
}

BOOST_AUTO_TEST_CASE(  test_omoms3_derivative )
{
	const double x = 0.2;
	CBSplineKernelOMoms3 kernel;
	CSplineKernel::VWeight weights(kernel.size());
	kernel.get_derivative_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		BOOST_CHECK_CLOSE(weights[3 - i], (omoms3( x - 2.0 + i + 0.0005) -
						   omoms3( x - 2.0 + i - 0.0005)) / 0.001 ,
				  0.01);
	}
}



// these tests need a review
#define TEST_HIGHORDER_DRIVATIVES 1
#ifdef TEST_HIGHORDER_DRIVATIVES

typedef bmpl::vector<CBSplineKernel2,
		     CBSplineKernel3,
		     CBSplineKernelOMoms3,
		     CBSplineKernel4,
		     CBSplineKernel5> test_kernels;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_derivatives, T, test_kernels)
{
	const double x = 0.2;
	T kernel;
	CSplineKernel::VWeight weights(kernel.size());
	CSplineKernel::VWeight pweights(kernel.size());
	CSplineKernel::VWeight mweights(kernel.size());
	kernel.get_derivative_weights(x, weights);
	kernel.get_weights(x + 0.00005, pweights);
	kernel.get_weights(x - 0.00005, mweights);

	for (size_t i = 0; i < weights.size(); ++i) {
		double test = (pweights[i] - mweights[i]) / 0.0001;
		BOOST_CHECK_CLOSE(weights[i], test, 0.01);
	}
}


typedef bmpl::vector<CBSplineKernel3,
		     CBSplineKernelOMoms3,
		     CBSplineKernel4,
		     CBSplineKernel5
		     > test_kernels2;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_derivatives2, T, test_kernels2)
{
	const double x = 0.2;
	T kernel;
	CSplineKernel::VWeight weights(kernel.size());
	CSplineKernel::VWeight pweights(kernel.size());
	CSplineKernel::VWeight mweights(kernel.size());
	CSplineKernel::VIndex index(kernel.size());

	kernel.derivative(x, weights, index, 2);
	kernel.get_derivative_weights(x + 0.00005, pweights);
	kernel.get_derivative_weights(x - 0.00005, mweights);

	for (size_t i = 0; i < weights.size(); ++i) {
		double test = (pweights[i] - mweights[i]) / 0.0001;
		BOOST_CHECK_CLOSE(weights[i], test, 0.01);
	}

}


typedef bmpl::vector<CBSplineKernel3,
		     CBSplineKernel4,
		     CBSplineKernel5
		     > test_kernels3;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_derivatives3, T, test_kernels3)
{
	const double x = 0.2;
	T kernel;
	vector<double> weights(kernel.size());
	vector<double> pweights(kernel.size());
	vector<double> mweights(kernel.size());
	CSplineKernel::VIndex index(kernel.size());

	kernel.derivative(x, weights, index, 3);
	kernel.get_derivative_weights(x + 0.00005, pweights, 2);
	kernel.get_derivative_weights(x - 0.00005, mweights, 2);

	for (size_t i = 0; i < weights.size(); ++i) {
		double test = (pweights[i] - mweights[i]) / 0.0001;
		BOOST_CHECK_CLOSE(weights[i], test, 0.01);
	}

}

#endif

BOOST_AUTO_TEST_CASE(test_half_size)
{
	BOOST_CHECK_EQUAL(CBSplineKernel2().get_active_halfrange(), 2); 
	BOOST_CHECK_EQUAL(CBSplineKernel3().get_active_halfrange(), 2); 
	BOOST_CHECK_EQUAL(CBSplineKernel4().get_active_halfrange(), 3); 
	BOOST_CHECK_EQUAL(CBSplineKernel5().get_active_halfrange(), 3); 
}


BOOST_AUTO_TEST_CASE(  test_spline1_get_weights )
{
	CBSplineKernel1 kernel;
	CSplineKernel::VWeight weight(2); 

	BOOST_CHECK_EQUAL(kernel.get_start_idx_and_value_weights(1.0, weight), 1); 
	BOOST_CHECK_EQUAL(weight[0], 1.0);
	BOOST_CHECK_EQUAL(weight[1], 0.0); 

	BOOST_CHECK_EQUAL(kernel.get_start_idx_and_value_weights(1.2, weight), 1); 
	BOOST_CHECK_CLOSE(weight[0], 0.8, 0.1);
	BOOST_CHECK_CLOSE(weight[1], 0.2, 0.1);

	BOOST_CHECK_EQUAL(kernel.get_start_idx_and_value_weights(2.5, weight), 2); 
	BOOST_CHECK_CLOSE(weight[0], 0.5, 0.1);
	BOOST_CHECK_CLOSE(weight[1], 0.5, 0.1);
	

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

	CSplineKernel::VWeight weight(4);
	CSplineKernel::VIndex index(4);
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

	kernel.derivative(-0.2, weight, index);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 1), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 1), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 1), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 1), weight[0], 0.1);


	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.5, 2) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.5, 2) + 1.0, 1.0, 0.1);

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.5, 2), 0.5, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.5, 2),-0.5, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(   0, 2),-2.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.5, 2),-0.5, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.5, 2), 0.5, 0.1);

	BOOST_CHECK_THROW(kernel.get_weight_at( 2, 3), invalid_argument);
}



BOOST_AUTO_TEST_CASE(  test_bspline3_weight_at_b )
{
	CBSplineKernel3 kernel;

	CSplineKernel::VWeight weight(4);
	CSplineKernel::VIndex index(4);
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
	CSplineKernel::VWeight weight(kernel.size());
	CSplineKernel::VIndex index(kernel.size());
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
	CSplineKernel::VWeight weight(kernel.size());
	CSplineKernel::VIndex  index(kernel.size());
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


BOOST_AUTO_TEST_CASE(  test_bspline4_weight_at )
{
	CBSplineKernel4 kernel;

	CSplineKernel::VWeight weight(5);
	CSplineKernel::VIndex index(5);
	kernel(0.0, weight, index);

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3, 0) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 3, 0) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2, 0), weight[0], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1, 0), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0, 0), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1, 0), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2, 0), weight[4], 0.1);

	kernel(-0.2, weight, index);

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 0), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 0), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 0), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 0), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 0), weight[0], 0.1);


	kernel.derivative(0.0, weight, index);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 1), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 1), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 1)+1, weight[2]+1, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 1), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 1), weight[0], 0.1);


	kernel.derivative(-0.2, weight, index);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 1), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 1), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 1), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 1), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 1), weight[0], 0.1);


	kernel.derivative(0.0, weight, index, 2);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.5, 2) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.5, 2) + 1.0, 1.0, 0.1);

	BOOST_CHECK_CLOSE(kernel.get_weight_at( -2, 2), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( -1, 2), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  0, 2), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  1, 2), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  2, 2), weight[0], 0.1);

	kernel.derivative(0.2, weight, index, 2);

	BOOST_CHECK_CLOSE(kernel.get_weight_at( -1.8, 2), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( -0.8, 2), weight[3], 0.1);


	BOOST_CHECK_CLOSE(kernel.get_weight_at(  0.2, 2), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  1.2, 2), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  2.2, 2), weight[0], 0.1);

	kernel.derivative(-0.2, weight, index, 2);

	BOOST_CHECK_CLOSE(kernel.get_weight_at( -2.2, 2), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( -1.2, 2), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( -0.2, 2), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  0.8, 2), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(  1.8, 2), weight[0], 0.1);



	BOOST_CHECK_THROW(kernel.get_weight_at( 2, 5), invalid_argument);
}

BOOST_AUTO_TEST_CASE(  test_bspline4_weights_d3 )
{
	CBSplineKernel4 kernel;

	CSplineKernel::VWeight weight(5);
	CSplineKernel::VIndex index(5);
	kernel.derivative(0.0, weight, index, 3);
	BOOST_CHECK_CLOSE(weight[4], kernel.get_weight_at(-2.0, 3), 0.1);
	BOOST_CHECK_CLOSE(weight[3], kernel.get_weight_at(-1.0, 3), 0.1);
	BOOST_CHECK_CLOSE(weight[2], kernel.get_weight_at( 0.0, 3), 0.1);
	BOOST_CHECK_CLOSE(weight[1], kernel.get_weight_at( 1.0, 3), 0.1);
	BOOST_CHECK_CLOSE(weight[0], kernel.get_weight_at( 2.0, 3), 0.1);

}

BOOST_AUTO_TEST_CASE(  test_bspline4_weight_at_d3 )
{
	CBSplineKernel4 kernel;
	for (double x = -3.0; x < 3.0; x += 0.3) {
		const double delta = (kernel.get_weight_at( x + 0.0001, 2) -
				      kernel.get_weight_at( x - 0.0001, 2)) / 0.0002;
		if (fabs(delta) < 0.001)
			BOOST_CHECK_CLOSE(kernel.get_weight_at(  x, 3) + 1, delta + 1, 0.1);
		else
			BOOST_CHECK_CLOSE(kernel.get_weight_at(  x, 3), delta, 0.1);
	}

}

BOOST_AUTO_TEST_CASE(  test_bspline4_weight_at_d4 )
{
	CBSplineKernel4 kernel;
	for (double x = -3.0; x < 3.0; x += 0.4) {
		const double delta = (kernel.get_weight_at( x + 0.0001, 3) -
				      kernel.get_weight_at( x - 0.0001, 3)) / 0.0002;
		if (fabs(delta) < 0.001)
			BOOST_CHECK_CLOSE(kernel.get_weight_at(  x, 4) + 1, delta + 1, 0.1);
		else
			BOOST_CHECK_CLOSE(kernel.get_weight_at(  x, 4), delta, 0.1);
	}

}

BOOST_AUTO_TEST_CASE(  test_bspline5_weight_at )
{
	CBSplineKernel5 kernel;

	CSplineKernel::VWeight weight(6);
	CSplineKernel::VIndex index(6);
	kernel(0.0, weight, index);

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3.5, 0) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 3.5, 0) + 1.0, 1.0, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 0), weight[0], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 0), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 0), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 0), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 0), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 3.0, 0), weight[5], 0.1);


	kernel(-0.2, weight, index);

	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 0), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 0), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 0), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 0), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 0), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.8, 0), weight[0], 0.1);



	kernel.derivative(0.0, weight, index);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3.0, 1), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 1), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 1), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 1)+1, weight[2]+1, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 1), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 1), weight[0], 0.1);




	kernel.derivative(-0.2, weight, index, 1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 1), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 1), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 1), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 1), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 1), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.8, 1), weight[0], 0.1);



	kernel.derivative(0.0, weight, index, 2);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3.0, 2), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 2), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 2), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 2)+1, weight[2]+1, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 2), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 2), weight[0], 0.1);




	kernel.derivative(-0.2, weight, index, 2);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 2), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 2), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 2), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 2), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 2), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.8, 2), weight[0], 0.1);


	kernel.derivative(0.0, weight, index, 3);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3.0, 3), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 3), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 3), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 3)+1, weight[2]+1, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 3), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 3), weight[0], 0.1);

	kernel.derivative(-0.2, weight, index, 3);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 3), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 3), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 3), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 3), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 3), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.8, 3), weight[0], 0.1);

#if 0
	kernel.derivative(0.0, weight, index, 4);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-3.0, 4), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.0, 4), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.0, 4), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.0, 4)+1, weight[2]+1, 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.0, 4), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.0, 4), weight[0], 0.1);

	kernel.derivative(-0.2, weight, index, 4);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-2.2, 4), weight[5], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-1.2, 4), weight[4], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at(-0.2, 4), weight[3], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 0.8, 4), weight[2], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 1.8, 4), weight[1], 0.1);
	BOOST_CHECK_CLOSE(kernel.get_weight_at( 2.8, 4), weight[0], 0.1);


	BOOST_CHECK_THROW(kernel.get_weight_at( 2, 5), invalid_argument);
#endif
}

BOOST_AUTO_TEST_CASE(  test_bspline3_integrate )
{
	CBSplineKernel3 kernel;
	BOOST_CHECK_CLOSE(integrate2(kernel, 10, 10, 1, 1, 1, 0, 30),  2.0/ 3.0, 0.1);
	BOOST_CHECK_CLOSE(integrate2(kernel, 10, 10, 2, 0, 1, 0, 30),  -2.0/ 3.0, 0.1);
	BOOST_CHECK_CLOSE(integrate2(kernel, 10, 11, 2, 0, 1, 0, 30),  0.125, 0.1);
	BOOST_CHECK_CLOSE(integrate2(kernel,  0,  1, 2, 0, 1, 0, 30),  0.1833, 2);
	BOOST_CHECK_CLOSE(integrate2(kernel, 29, 27, 0, 2, 1, 0, 30),  0.2, 2);
}

BOOST_AUTO_TEST_CASE(  test_bspline4_integrate )
{
	CBSplineKernel4 kernel;
	BOOST_CHECK_CLOSE(integrate2(kernel, 10, 10, 1, 1, 1, 0, 30),  0.4861, 0.1);
	BOOST_CHECK_CLOSE(integrate2(kernel, 10, 10, 2, 0, 1, 0, 30),  -0.4861, 0.1);
}


BOOST_AUTO_TEST_CASE(  test_bspline4_sum_integrate_11 )
{

	CBSplineKernel4 kernel;
	
	double d4a = integrate2(kernel, 10, 6, 1, 1, 1, 0, 30); 
	double d3a = integrate2(kernel, 10, 7, 1, 1, 1, 0, 30); 
	double d2a = integrate2(kernel, 10, 8, 1, 1, 1, 0, 30); 
	double d1a = integrate2(kernel, 10, 9, 1, 1, 1, 0, 30); 
	double d0 = integrate2(kernel, 10,10, 1, 1, 1, 0, 30); 
	double d1b = integrate2(kernel, 10,11, 1, 1, 1, 0, 30); 
	double d2b = integrate2(kernel, 10,12, 1, 1, 1, 0, 30); 
	double d3b = integrate2(kernel, 10,13, 1, 1, 1, 0, 30); 
	double d4b = integrate2(kernel, 10,14, 1, 1, 1, 0, 30); 
	
	cvdebug() << "d4a = " << d4a << "\n";  
	cvdebug() << "d3a = " << d3a << "\n";  
	cvdebug() << "d2a = " << d2a << "\n";  
	cvdebug() << "d1a = " << d1a << "\n";  
	cvdebug() << "d0  = " << d0  << "\n";  
	cvdebug() << "d1b = " << d1b << "\n";  
	cvdebug() << "d2b = " << d2b << "\n";  
	cvdebug() << "d3b = " << d3b << "\n";  
	cvdebug() << "d4b = " << d4b << "\n";  

	double sum = d4a + d4b + d3a + d3b + d2a + d2b + d1a + d1b + d0; 

	BOOST_CHECK_CLOSE(1.0 + sum, 1.0, 0.1); 
}

BOOST_AUTO_TEST_CASE(  test_bspline4_equivalence )
{
	CBSplineKernel3 kernel;
	
	auto bc = produce_spline_boundary_condition("mirror"); 
	bc->set_width(10); 
	CSplineKernel::SCache cache(kernel.size(), *bc, false); 
	CSplineKernel::VWeight weights(kernel.size()); 
	CSplineKernel::VIndex indices(kernel.size()); 

	double x1 = 1.4; 
	
	kernel(x1, cache); 
	kernel(x1, weights, indices); 

	BOOST_CHECK_EQUAL(cache.weights[0], weights[0]); 
	BOOST_CHECK_EQUAL(cache.weights[1], weights[1]); 
	BOOST_CHECK_EQUAL(cache.weights[2], weights[2]); 
	BOOST_CHECK_EQUAL(cache.weights[3], weights[3]); 
	
	if (!cache.is_flat) {
		BOOST_CHECK_EQUAL(cache.index[0], indices[0]); 
		BOOST_CHECK_EQUAL(cache.index[1], indices[1]); 
		BOOST_CHECK_EQUAL(cache.index[2], indices[2]); 
		BOOST_CHECK_EQUAL(cache.index[3], indices[3]); 
	}else{
		BOOST_CHECK_EQUAL(cache.start_idx, indices[0]); 
	}
}


BOOST_AUTO_TEST_CASE(  test_bspline3_uncached_flat )
{
	CBSplineKernel3 kernel;
	
	auto bc = produce_spline_boundary_condition("mirror"); 
	bc->set_width(10); 
	CSplineKernel::SCache cache(kernel.size(), *bc, false); 
	CSplineKernel::VWeight weights(kernel.size()); 
	CSplineKernel::VIndex indices(kernel.size()); 

	double x1 = 2.4; 
	
	kernel.get_uncached(x1, cache); 
	kernel(x1, weights, indices); 

	BOOST_CHECK_EQUAL(cache.weights[0], weights[0]); 
	BOOST_CHECK_EQUAL(cache.weights[1], weights[1]); 
	BOOST_CHECK_EQUAL(cache.weights[2], weights[2]); 
	BOOST_CHECK_EQUAL(cache.weights[3], weights[3]); 
	
	BOOST_CHECK(cache.is_flat); 
	BOOST_CHECK_EQUAL(cache.start_idx, indices[0]); 
}

BOOST_AUTO_TEST_CASE(  test_bspline3_uncached_nonflat )
{
	CBSplineKernel3 kernel;
	
	auto bc = produce_spline_boundary_condition("mirror"); 
	bc->set_width(10); 
	CSplineKernel::SCache cache(kernel.size(), *bc, false); 
	CSplineKernel::VWeight weights(kernel.size()); 
	CSplineKernel::VIndex indices(kernel.size()); 

	double x1 = 0.4; 
	
	kernel.get_uncached(x1, cache); 
	kernel(x1, weights, indices); 

	BOOST_CHECK_EQUAL(cache.weights[0], weights[0]); 
	BOOST_CHECK_EQUAL(cache.weights[1], weights[1]); 
	BOOST_CHECK_EQUAL(cache.weights[2], weights[2]); 
	BOOST_CHECK_EQUAL(cache.weights[3], weights[3]); 
	
	BOOST_CHECK(!cache.is_flat); 
	BOOST_CHECK_EQUAL(cache.index[0],  1); 
	BOOST_CHECK_EQUAL(cache.index[1],  0); 
	BOOST_CHECK_EQUAL(cache.index[2],  1); 
	BOOST_CHECK_EQUAL(cache.index[3],  2); 
}

BOOST_AUTO_TEST_CASE(  test_bspline3_cached )
{
	CBSplineKernel3 kernel;
	
	auto bc = produce_spline_boundary_condition("mirror"); 
	bc->set_width(10); 
	CSplineKernel::SCache cache(kernel.size(), *bc, false); 
	CSplineKernel::VWeight weights(kernel.size()); 
	CSplineKernel::VIndex indices(kernel.size()); 

	double x1 = 2.4; 
	
	kernel.get_cached(x1, cache); 
	kernel(x1, weights, indices); 

	BOOST_CHECK_EQUAL(cache.weights[0], weights[0]); 
	BOOST_CHECK_EQUAL(cache.weights[1], weights[1]); 
	BOOST_CHECK_EQUAL(cache.weights[2], weights[2]); 
	BOOST_CHECK_EQUAL(cache.weights[3], weights[3]); 
	
	BOOST_CHECK(!cache.is_flat); 
	BOOST_CHECK_EQUAL(cache.index[0], indices[0]); 
	BOOST_CHECK_EQUAL(cache.index[1], indices[1]); 
	BOOST_CHECK_EQUAL(cache.index[2], indices[2]); 
	BOOST_CHECK_EQUAL(cache.index[3], indices[3]); 

	kernel.get_cached(x1, cache); 

	BOOST_CHECK_EQUAL(cache.weights[0], weights[0]); 
	BOOST_CHECK_EQUAL(cache.weights[1], weights[1]); 
	BOOST_CHECK_EQUAL(cache.weights[2], weights[2]); 
	BOOST_CHECK_EQUAL(cache.weights[3], weights[3]); 
	
	BOOST_CHECK(!cache.is_flat); 
	BOOST_CHECK_EQUAL(cache.index[0], indices[0]); 
	BOOST_CHECK_EQUAL(cache.index[1], indices[1]); 
	BOOST_CHECK_EQUAL(cache.index[2], indices[2]); 
	BOOST_CHECK_EQUAL(cache.index[3], indices[3]); 

}

