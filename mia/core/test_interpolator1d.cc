/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2009 -.2010
 *
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

#include <mia/internal/autotest.hh>
#include <mia/core/interpolator1d.hh>

#include <cmath>
#include <memory>

using namespace std; 
using namespace mia; 

struct InterpolatorIDFixture  {
	
	double f(double x) const; 
	
	double df(double x) const; 

	void test_case(EInterpolation type, double tolerance = 1.0); 

}; 


BOOST_FIXTURE_TEST_CASE( test_linear, InterpolatorIDFixture) 
{

	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(ip_linear)); 
	vector<double> data(512); 
	

	for(size_t x = 0; x < 256; ++x) 
		data[x] = f(M_PI * x / 511.0) ; 
	
	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data)); 
	BOOST_CHECK_CLOSE( (*interp)(0) + 1, f(0.0) + 1, 1.0); 
	for(size_t i = 20; i < 100; ++i) {
		double x = M_PI * i / 511.0; 
		BOOST_CHECK_CLOSE( (*interp)(i), f(x), 1.0); 
		BOOST_CHECK_CLOSE( 511.0 / M_PI * interp->derivative_at(i), df(x), 1.0);
	}

}

BOOST_FIXTURE_TEST_CASE( test_bspline2, InterpolatorIDFixture) 
{
	test_case(ip_bspline2); 
}

BOOST_FIXTURE_TEST_CASE( test_bspline3, InterpolatorIDFixture) 
{
	test_case(ip_bspline3); 
}

BOOST_FIXTURE_TEST_CASE( test_bspline4, InterpolatorIDFixture) 
{
	test_case(ip_bspline4); 
}

BOOST_FIXTURE_TEST_CASE( test_bspline5, InterpolatorIDFixture) 
{
	test_case(ip_bspline5); 
}


BOOST_FIXTURE_TEST_CASE( test_omoms3, InterpolatorIDFixture) 
{
	test_case(ip_omoms3); 
}





double InterpolatorIDFixture::f(double x) const
{
	return sin(x); 
}

double InterpolatorIDFixture::df(double x) const
{
	return cos(x); 
}

void InterpolatorIDFixture::test_case(EInterpolation type, double tolerance)
{
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(type)); 
	vector<double> data(512); 
	

	for(size_t x = 0; x < 511; ++x) 
		data[x] = f(M_PI * x / 511.0) ; 
	
	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data)); 
	BOOST_CHECK_CLOSE( (*interp)(0) + 1, f(0.0) + 1, tolerance); 
	for(size_t i = 20; i < 500; ++i) {
		double x = M_PI * i / 511.0; 
		BOOST_CHECK_CLOSE( (*interp)(i), f(x), tolerance); 
		BOOST_CHECK_CLOSE( 511.0 / M_PI * interp->derivative_at(i), df(x), tolerance);
	}

}
