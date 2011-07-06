/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

struct Scaler1DFixture  {
	Scaler1DFixture(); 

	double f(double x) const;
	void test_size(EInterpolation type, size_t target_size);
	gsl::DoubleVector data; 
};


BOOST_FIXTURE_TEST_CASE( test_bspline2_upscale, Scaler1DFixture)
{
	test_size(ip_bspline2, 500);
}

BOOST_FIXTURE_TEST_CASE( test_bspline2_downscale, Scaler1DFixture)
{
	test_size(ip_bspline2, 130);
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
	data(256)
{

	list< bfs::path> sksearchpath; 
	sksearchpath.push_back( bfs::path("splinekernel"));
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 

	for(size_t x = 0; x < 256; ++x)
		data[x] = 200*f(2 * M_PI * x / 255.0);
}

void Scaler1DFixture::test_size(EInterpolation type, size_t target_size)
{
	gsl::DoubleVector result(target_size); 
	
	unique_ptr<C1DInterpolatorFactory>  ipf(create_1dinterpolation_factory(type, bc_mirror_on_bounds));	
	C1DScalarFixed scaler(*ipf->get_kernel(), data.size(), target_size); 
	copy(data.begin(), data.end(), scaler.input_begin()); 
	
	scaler.run(); 
	
	copy(scaler.output_begin(), scaler.output_end(), result.begin()); 

	for(size_t i = 0; i < target_size; ++i) {
		double x = (2 * M_PI * i) / (target_size - 1); 
		double fx = 200*f(x); 
		cvdebug()  << " sin("<< x << ") = " << fx 
			   << ", interp= " << result[i] 
			   << ", Q= " << fx/ result[i] 
			   << "\n"; 

		
		if (abs(fx) > 0.0001 || abs(result[i]) > 0.0001) 
			BOOST_CHECK_CLOSE( fx, result[i], 0.1);
	}
}

