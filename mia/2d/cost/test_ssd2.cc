/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
\author Gert Wollny <wollny at die.upm.ed>

*/

// the actual implementation is here, shared between
// 2d and 3d
#include <mia/internal/autotest.hh>
#include <mia/2d/transformmock.hh>
#include <mia/2d/cost/ssd2.hh>


using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;
using namespace mia_2dcost_ssd2;


struct SSD2DFixture {

	SSD2DFixture();

	C2DBounds size;
	string src_key;
	string ref_key;
	P2DInterpolatorFactory ipf;
	C2DFVectorfield force;
	C2DTransformMock t;
};


BOOST_FIXTURE_TEST_CASE( test_SSD_2D, SSD2DFixture )
{
	C2DSSDImageCost cost(src_key, ref_key, ipf, 1.0);
	double cost_value = cost.evaluate(t, force);
	BOOST_CHECK_CLOSE(cost_value, 55.0 / 32.0, 0.1);
	BOOST_CHECK_CLOSE(force(1,1).x, 0.5f, 0.1);
	BOOST_CHECK_CLOSE(force(1,1).y, 3.0f, 0.1);
}


BOOST_FIXTURE_TEST_CASE( test_SSD_2D_scaled, SSD2DFixture )
{
	C2DSSDImageCost cost(src_key, ref_key, ipf, 0.5);
	double cost_value = cost.evaluate(t, force);
	BOOST_CHECK_CLOSE(cost_value, 55.0 / 64.0, 0.1);
	BOOST_CHECK_CLOSE(force(1,1).x, 0.25f, 0.1);
	BOOST_CHECK_CLOSE(force(1,1).y, 1.5f, 0.1);
}


SSD2DFixture::SSD2DFixture():
	size(4,4),
	src_key("src"),
	ref_key("ref"),
	ipf(create_2dinterpolation_factory(ip_linear)),
	force(size),
	t(size)
{
	const float src_data[16] = {
		0, 0, 0, 0,
		0, 3, 1, 0,
		0, 6, 7, 0,
		0, 0, 0, 0
	};
	const float ref_data[16] = {
		0, 0, 0, 0,
		0, 2, 3, 0,
		0, 1, 2, 0,
		0, 0, 0, 0
	};

	P2DImage src(new C2DFImage(size, src_data ));
	P2DImage ref(new C2DFImage(size, ref_data ));

	CDatapool::Instance().add(src_key, create_image2d_vector(src));
	CDatapool::Instance().add(ref_key, create_image2d_vector(ref));

}

