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

#include <mia/internal/autotest.hh>
#include <mia/2d/cost/fatnssd.hh>

using namespace std;
using namespace boost;
namespace bfs=::boost::filesystem;
using namespace mia;

using namespace nssd_2dimage_fatcost;

struct NSSD2DFixture {
	NSSD2DFixture() {
		cvdebug() << "Init fixture\n";
		list< bfs::path> plugpath;
		plugpath.push_back(bfs::path("./"));
		C2DImageCostPluginHandler::set_search_path(plugpath);
	}
};

BOOST_FIXTURE_TEST_CASE( test_NSSD2D_self, NSSD2DFixture )
{
	const C2DBounds size(10,20);
	C2DUBImage *img = new C2DUBImage(size);
	C2DUBImage::iterator isrc = img->begin();
	for (size_t i = 0; i < size.x * size.y; ++i, ++isrc)
		*isrc = i;

	P2DImage src(new C2DUBImage(size));

	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3));
	CFatNSSD2DImageCost cost(src, src, ipf, 1.0);

	BOOST_CHECK_CLOSE(1.0 + cost.value(), 1.0, 0.1);

	C2DFVectorfield force(size);
	double c = cost.evaluate_force(force);
	BOOST_CHECK_CLOSE(1.0 + c, 1.0, 0.1);

	C2DFVectorfield::const_iterator i = force.begin();

	while (i != force.end()) {
		BOOST_CHECK_CLOSE(1.0 + i->x, 1.0, 0.1);
		BOOST_CHECK_CLOSE(1.0 + i->y, 1.0, 0.1);
		++i;
	}
}

BOOST_FIXTURE_TEST_CASE( test_SSD2D_flat_images, NSSD2DFixture )
{
	const C2DBounds size(10,20);
	C2DUBImage *psrc = new C2DUBImage(size);
	C2DUBImage *pref = new C2DUBImage(size);

	fill(psrc->begin(), psrc->end(), 1.0);
	fill(pref->begin(), pref->end(), 2.0);

	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3));

	CFatNSSD2DImageCost cost(P2DImage(psrc), P2DImage(pref), ipf, 2.0);
	BOOST_CHECK_CLOSE(1.0 + cost.value(), 1.0, 0.1);

	C2DFVectorfield force(size);
	double c = cost.evaluate_force(force);
	BOOST_CHECK_CLOSE(1.0 + c, 1.0, 0.1);

	const C2DFVector zero(0,0);

	C2DFVectorfield::const_iterator i = force.begin();
	while (i != force.end()) {
		BOOST_CHECK_EQUAL(*i, zero);
		++i;
	}

}


BOOST_FIXTURE_TEST_CASE( test_SSD2D, NSSD2DFixture )
{
	const C2DBounds size(3,3);

	const unsigned char src_init[9] = {1, 2, 3,   3, 2, 2,   1, 3, 1 };
	const unsigned char ref_init[9] = {2, 1, 4,   2, 1, 1,   2, 3, 2 };

	C2DUBImage *psrc = new C2DUBImage(size, src_init);
	C2DUBImage *pref = new C2DUBImage(size, ref_init);

	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3));
	CFatNSSD2DImageCost cost(P2DImage(psrc), P2DImage(pref), ipf, 2.0);
	BOOST_CHECK_CLOSE(cost.value(), 2.0 * 1.00797741, 0.1);

	C2DFVectorfield force(size);
	double c = cost.evaluate_force(force);
	BOOST_CHECK_CLOSE(c, 2.0 * 1.00797741, 0.1);

	const C2DFVector zero(0,0);

	C2DFVectorfield::const_iterator i = force.begin();
	C2DFVector rforce = force(1,1);

	BOOST_CHECK_CLOSE(rforce.x , 2.0f * -0.57735f, 0.1);
	BOOST_CHECK_CLOSE(rforce.y , 2.0f *  0.57735f, 0.1);

}
