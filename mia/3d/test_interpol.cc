/*
** Copyrigh (C) 2004 MPI of Human Cognitive and Brain Sience
**                    Gert Wollny <wollny at cbs.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   
*/
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/3d/interpolator.hh>
#include <mia/core/msgstream.hh>

NS_MIA_USE
using namespace std; 
using namespace boost; 
	
template <class Data3D, class Interpolator>
bool test_interpolator(const Data3D& data, const Interpolator& src)
{
	bool result = true; 
	
	typename Data3D::const_iterator i = data.begin();
	
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i) {
				C3DFVector loc(x,y,z); 
				typename Data3D::value_type v = src(loc); 
				double delta = v - *i; 
				if ( fabs(delta) > 1e-4 ) {
					cvdebug() << loc << " fails: (" << v <<") vs. ("  << *i << ")\t ->  delta = " << v-*i <<   std::endl; 					result =  false; 
				}
			}
	return result; 
}


template <class T, template <class> class Interpolator>
static bool test_direct_interpolator(const T3DDatafield<T>& data)
{
	Interpolator<T> src(data);
	return test_interpolator(data, src);
}


template <class T>
static bool test_conv_interpolator(const T3DDatafield<T>& data, SHARED_PTR(CBSplineKernel) kernel)
{
	T3DConvoluteInterpolator<T>  src(data, kernel);
	bool result = test_interpolator(data, src);
	return result; 
}

template <class T>
static void test_type() 
{
	T3DDatafield<T> data(C3DBounds(10, 12, 11));
	
	typename T3DDatafield<T>::iterator i = data.begin(); 
	for (size_t z = 0; z < data.get_size().z; ++z)
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++i) 
				*i = z; 
	

	if (!test_direct_interpolator<T, T3DNNInterpolator>(data))
		BOOST_FAIL(" NN Interpolator FAIL"); 

	if (!test_direct_interpolator<T, T3DTrilinearInterpolator>(data))
		BOOST_FAIL(" Tri Interpolator FAIL"); 
	
	if (!test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel) (new CBSplineKernel2())))
		BOOST_FAIL(" BSpline 2 Interpolator FAIL");
	
	if (!test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel) (new CBSplineKernel3())))
		BOOST_FAIL(" BSpline 3 Interpolator FAIL"); 
	
	if (!test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel) (new CBSplineKernel4())))
		BOOST_FAIL(" BSpline 4 Interpolator FAIL"); 
	
	if (!test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel) (new CBSplineKernel5())))
		BOOST_FAIL(" BSpline 5 Interpolator FAIL\n"); 
	
	if (!test_conv_interpolator<T>(data, SHARED_PTR(CBSplineKernel) (new CBSplineKernelOMoms3())))
		BOOST_FAIL(" oMoms 3 Interpolator FAIL\n"); 
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
	CBSplineKernelOMoms3 kernel; 
	std::vector<double> weights(kernel.size());
	kernel.get_weights(x, weights); 
	
	for (size_t i = 0; i < weights.size(); ++i) {
		if (fabs(weights[3 - i] - omoms3( x - 2.0 + i)) > 1e-4)
			cvfail() << "omoms coeff FAILED" << std::endl;  
	}
}

void add_3dinterpol_tests( boost::unit_test::test_suite* suite)
{
	suite->add( BOOST_TEST_CASE( &test_omoms3)); 
	suite->add( BOOST_TEST_CASE( &test_type<unsigned char>));
	suite->add( BOOST_TEST_CASE( &test_type<signed char>));
	suite->add( BOOST_TEST_CASE( &test_type<unsigned short>));
	suite->add( BOOST_TEST_CASE( &test_type<signed short>));
#ifdef HAVE_INT64
	suite->add( BOOST_TEST_CASE( &test_type<mia_int64>));
	suite->add( BOOST_TEST_CASE( &test_type<mia_uint64>));
#endif
	suite->add( BOOST_TEST_CASE( &test_type<float>));
	suite->add( BOOST_TEST_CASE( &test_type<double>));
}
