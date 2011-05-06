/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

	void test_case(EInterpolation type, double tolerance = 0.1);

};


BOOST_FIXTURE_TEST_CASE( test_linear, InterpolatorIDFixture)
{

	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(ip_linear));
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
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(type));
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

