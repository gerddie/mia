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

#include <stdexcept>
#include <climits>
#include <cmath>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/spacial_kernel.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>


#include <mia/core/scaler1d.hh>


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

CSplineBoundaryConditionTestPath bc_path; 

struct Scaler1DFixture  {
	Scaler1DFixture(); 

	double f(double x) const;
	void test_size(EInterpolation type, size_t target_size);
	void test_scale_by_factor(const string& kernel_descr, double scale, size_t expected_size);


	C1DScalar::std_double_vector data; 
};


BOOST_FIXTURE_TEST_CASE( test_bspline2_upscale, Scaler1DFixture)
{
	test_size(ip_bspline2, 500);
}

BOOST_FIXTURE_TEST_CASE( test_bspline2_upscale_scale, Scaler1DFixture)
{
	test_scale_by_factor("bspline:d=2", 2.5, 639);
}

BOOST_FIXTURE_TEST_CASE( test_bspline2_downscale_scale, Scaler1DFixture)
{
	test_scale_by_factor("bspline:d=2", 0.5, 129);
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_upscale_scale, Scaler1DFixture)
{
	test_scale_by_factor("bspline:d=3", 1.452, 372);
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_downscale_scale, Scaler1DFixture)
{
	test_scale_by_factor("bspline:d=3", 0.4671, 121);
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_upscale, Scaler1DFixture)
{
	test_size(ip_bspline3, 500);
}

BOOST_FIXTURE_TEST_CASE( test_bspline3_downscale, Scaler1DFixture)
{
	test_size(ip_bspline3, 130);
}


BOOST_FIXTURE_TEST_CASE( test_bspline4_upscale, Scaler1DFixture)
{
	test_size(ip_bspline4, 500);
}

BOOST_FIXTURE_TEST_CASE( test_bspline4_downscale, Scaler1DFixture)
{
	test_size(ip_bspline4, 130);
}


BOOST_FIXTURE_TEST_CASE( test_bspline5_upscale, Scaler1DFixture)
{
	test_size(ip_bspline5, 500);
}

BOOST_FIXTURE_TEST_CASE( test_bspline5_downscale, Scaler1DFixture)
{
	test_size(ip_bspline5, 130);
}


BOOST_FIXTURE_TEST_CASE( test_omoms3_upscale, Scaler1DFixture)
{
	test_size(ip_omoms3, 500);
}
BOOST_FIXTURE_TEST_CASE( test_omoms3_downscale, Scaler1DFixture)
{
	test_size(ip_omoms3, 130);
}


double Scaler1DFixture::f(double x) const
{
	return cos(x);
}

Scaler1DFixture::Scaler1DFixture():
	data(256, false)
{

	CPathNameArray  sksearchpath({bfs::path("splinekernel")});
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 
	
	const double intervall = 2 * M_PI / 255.0; 

	for(size_t x = 0; x < 256; ++x)
		data[x] = 200*f(intervall * x);
}

void Scaler1DFixture::test_size(EInterpolation type, size_t target_size)
{
	C1DScalar::std_double_vector result(target_size); 
	
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(type, bc_mirror_on_bounds));	
	C1DScalar scaler(*ipf->get_kernel(), data.size(), target_size); 

	copy(data.begin(), data.end(), scaler.input_begin()); 
	
	scaler.run(); 
	
	copy(scaler.output_begin(), scaler.output_end(), result.begin()); 
	
	const double intervall = 2 * M_PI / (target_size - 1); 

	for(size_t i = 0; i < target_size; ++i) {
		double x = intervall * i; 
		double fx = 200*f(x); 
		cvdebug()  << " sin("<< x << ") = " << fx 
			   << ", interp= " << result[i] 
			   << ", Q= " << fx/ result[i] 
			   << "\n"; 

		
		if (abs(fx) > 0.0001 || abs(result[i]) > 0.0001) 
			BOOST_CHECK_CLOSE( fx, result[i], 0.1);
	}
}

void Scaler1DFixture::test_scale_by_factor(const string& kernel_descr, double scale, size_t expected_size)
{
	auto kernel = produce_spline_kernel(kernel_descr); 
	
	C1DScalar scaler(*kernel, data.size(), scale); 

	BOOST_CHECK_EQUAL(scaler.get_output_size(), expected_size); 
	C1DScalar::std_double_vector result(expected_size); 
	
	copy(data.begin(), data.end(), scaler.input_begin()); 
	scaler.run(); 
	copy(scaler.output_begin(), scaler.output_end(), result.begin()); 

        double test_scale = 2 * M_PI * 1.0 / scale / 255.0; 

	for(size_t i = 0; i < expected_size; ++i) {
		double x = test_scale * i;
		double fx = 200*f(x); 
		cvdebug()  << " sin("<< x << ") = " << fx 
			   << ", interp= " << result[i] 
			   << ", Q= " << fx/ result[i] 
			   << "\n"; 

		
		if (abs(fx) > 0.0001 || abs(result[i]) > 0.0001) 
			BOOST_CHECK_CLOSE( fx, result[i], 0.1);
	}

}
