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

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/core/spacial_kernel.hh>
#include <mia/2d/interpolator.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

CSplineKernelTestPath init_splinekernel_path; 

struct FieldSplineFixture {
	FieldSplineFixture():
		size(30, 32),
		field(size),
		ipf(ipf_spline, CSplineKernelPluginHandler::instance().produce("bspline:d=4"))
	{
		C2DFVectorfield::iterator i = field.begin();
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				*i = C2DFVector( fx(x, y), fy(x, y));
			}

		std::shared_ptr<T2DInterpolator<C2DFVector>  > source(ipf.create(field));

		list< bfs::path> kernelsearchpath;
		kernelsearchpath.push_back(bfs::path("../core/spacialkernel"));
		C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);

	}
	C2DBounds size;
	C2DFVectorfield field;
	C2DInterpolatorFactory ipf;

protected:
	float fx(float x, float y);
	float fy(float x, float y);
};

float FieldSplineFixture::fx(float x, float y)
{
	x /= 25.0;
	y /= 50.0;
	return (x + 1) / (x +  y  * x + 2);
}

float FieldSplineFixture::fy(float x, float y)
{
	x /= 25.0;
	y /= 50.0;
	return (y + 1) / (x * x + y + 10);
}

BOOST_FIXTURE_TEST_CASE( test_vector_field2d_splines, FieldSplineFixture )
{
	std::shared_ptr<T2DInterpolator<C2DFVector>  > source(ipf.create(field));

	C2DFVector result = (*source)(C2DFVector(5.4, 23.5));

	BOOST_CHECK_CLOSE(result.x, fx(5.4, 23.5), 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(5.4, 23.5), 0.1);

}


BOOST_AUTO_TEST_CASE( test_vf_cat )
{

	C2DFVector init_a[9] = {
		C2DFVector(2,3), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(-1,-1), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0)
	};

	C2DFVector init_b[9] = {
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(1,1), C2DFVector(0.5,0),
		C2DFVector(0,0), C2DFVector(0,0.5), C2DFVector(4,5)
	};


	C2DBounds size(3,3);
	C2DFVectorfield a(size);
	C2DFVectorfield b(size);

	copy(init_a, init_a + 9, a.begin());
	copy(init_b, init_b + 9, b.begin());

	a += b;

	BOOST_CHECK( a(1,1) == C2DFVector(3,4));

	BOOST_CHECK( a(2,1) == C2DFVector(0.0,-0.5));
	BOOST_CHECK( a(1,2) == C2DFVector(-0.5,0.0));

}


