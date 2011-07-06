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

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/spacial_kernel.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/scale1d.hh>


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

class ScaleFixtureBase {

protected:
	ScaleFixtureBase();
	void initialize(size_t src_size, const float *src_init, size_t test_size, const float *test_init);

	void check_result(const vector<float>& result);
	vector<float> src;
	vector<float> test;


};

BOOST_FIXTURE_TEST_CASE( test_upscale_NN, ScaleFixtureBase)
{
	const size_t in_size = 3;
	const float init_src[in_size] = {2, 4, 7};
	const size_t test_size = 6;
	const float init_test[test_size] = {2, 2, 4, 4, 7, 7};

	initialize( in_size, init_src, test_size, init_test);

	C1DScalar scaler(P1DInterpolatorFactory(create_1dinterpolation_factory(ip_bspline0,bc_mirror_on_bounds)));
	vector<float> result(test_size);
	scaler(src, result);
	check_result(result);
}

BOOST_FIXTURE_TEST_CASE( test_upscale_linear, ScaleFixtureBase)
{
	const size_t in_size = 3;
	const float init_src[in_size] = {2, 4, 7};
	const size_t test_size = 5;
	const float init_test[test_size] = {2, 3, 4, 5.5, 7};

	initialize( in_size, init_src, test_size, init_test);

	C1DScalar scaler(P1DInterpolatorFactory(create_1dinterpolation_factory(ip_bspline1, bc_mirror_on_bounds)));
	vector<float> result(test_size);
	scaler(src, result);
	check_result(result);
}

BOOST_FIXTURE_TEST_CASE( test_downscale_linear, ScaleFixtureBase)
{
	const size_t test_size = 3;
	const float init_test[test_size] = {6, 5, 5};
	const size_t src_size = 5;
	const float init_src[src_size] = {4, 8, 4, 4, 8};

	initialize( src_size, init_src, test_size, init_test);

	C1DScalar scaler(P1DInterpolatorFactory(create_1dinterpolation_factory(ip_bspline1, bc_mirror_on_bounds)));
	vector<float> result(test_size);
	scaler(src, result);
	check_result(result);
}

ScaleFixtureBase::ScaleFixtureBase()
{
	list< bfs::path> sksearchpath; 
	sksearchpath.push_back( bfs::path("splinekernel"));
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 


	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);
}

void ScaleFixtureBase::initialize(size_t src_size, const float *src_init,
				   size_t test_size, const float *test_init)
{
	src.resize(src_size);
	test.resize(test_size);
	copy(src_init, src_init + src_size, src.begin());
	copy(test_init, test_init + test_size, test.begin());
}

void ScaleFixtureBase::check_result(const vector<float>& result)
{
	BOOST_CHECK_EQUAL(test.size(), result.size());
	for (size_t i = 0; i < test.size(); ++i)
		BOOST_CHECK_CLOSE(test[i], result[i], 0.1);
}
