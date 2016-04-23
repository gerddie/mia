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

#include <mia/internal/autotest.hh>
#include <mia/core/interpolator1d.hh>
#include <boost/filesystem.hpp>

#include <cmath>
#include <memory>

using namespace std;
using namespace mia;
namespace bfs=boost::filesystem; 

struct InterpolatorIDFixture  {

	double f(double x) const;

	double df(double x) const;

	void test_case(EInterpolation type, double tolerance = 0.1);

};

BOOST_FIXTURE_TEST_CASE( test_linear, InterpolatorIDFixture)
{

	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(ip_linear, bc_mirror_on_bounds));
	vector<double> data(512);


	for(size_t x = 0; x < 256; ++x)
		data[x] = f(M_PI * x / 512.0) ;

	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data));
	BOOST_CHECK_CLOSE( (*interp)(0) + 1, f(0.0) + 1, 1.0);
	for(size_t i = 20; i < 100; ++i) {
		double x = M_PI * i / 512.0;
		BOOST_CHECK_CLOSE( (*interp)(i), f(x), 1.0);
		BOOST_CHECK_CLOSE( 512.0 / M_PI * interp->derivative_at(i), df(x), 1.0);
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
	return sin(M_PI * x / 512.0);
}

double InterpolatorIDFixture::df(double x) const
{
	return M_PI / 512.0 * cos(M_PI * x / 512.0);
}


void InterpolatorIDFixture::test_case(EInterpolation type, double tolerance)
{
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(type,bc_mirror_on_bounds));
	vector<double> data(512); 
	for(size_t x = 0; x < 512; ++x)
		data[x] = f(x) ;

	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data));
	BOOST_CHECK_CLOSE( (*interp)(0) + 1, f(0.0) + 1, tolerance);
	for(size_t x = 0; x < 512; ++x) {
		double interpx = (*interp)(x);
		if (abs(interpx > 1e-10) || f(x) >1e-10)
			BOOST_CHECK_CLOSE( interpx, f(x), tolerance);
	}

	// the filtering preparation of the data makes the gradined
	// at the boundaries unreliable 
	for(size_t x = 10; x < 502; ++x) {
		double interpdx = interp->derivative_at(x); 
		if (abs(interpdx > 1e-3) || df(x) >1e-3)
			BOOST_CHECK_CLOSE( interpdx, df(x), 2*tolerance);
	}
}



BOOST_AUTO_TEST_CASE( test_bspline0_zero )
{
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(ip_bspline0,bc_zero));

	vector<double> data(10);
	for(size_t x = 0; x < 10; ++x)
		data[x] = x - 1 ;

	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data));

	BOOST_CHECK_EQUAL( (*interp)(-1), 0.0);
	BOOST_CHECK_EQUAL( (*interp)(11), 0.0);

	for(size_t x = 0; x < 10; ++x) {
		
		BOOST_CHECK_EQUAL( (*interp)(x - 0.2), x - 1);
		BOOST_CHECK_EQUAL( (*interp)(x + 0.2), x - 1);
	
	}
}

BOOST_AUTO_TEST_CASE( test_bspline0_repeat )
{
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(ip_bspline0,bc_repeat));

	vector<double> data(10);
	for(size_t x = 0; x < 10; ++x)
		data[x] = x +1 ;

	unique_ptr< T1DInterpolator<double> > interp(ipf->create(data));

	BOOST_CHECK_EQUAL( (*interp)(-1), 1.0);
	BOOST_CHECK_EQUAL( (*interp)(11), 10.0);

	for(size_t x = 0; x < 10; ++x) {
		
		BOOST_CHECK_EQUAL( (*interp)(x - 0.2), x + 1);
		BOOST_CHECK_EQUAL( (*interp)(x + 0.2), x + 1);
	
	}
}
