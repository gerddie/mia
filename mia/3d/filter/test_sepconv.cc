/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
#include <mia/3d/filter/sepconv.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace sepconv_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_sepconv )
{

	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("..")/bfs::path("core")/bfs::path("spacialkernel"));
	C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);

	C3DFImage src(C3DBounds(3,3,3));
	fill(src.begin(), src.end(), 0);
	src(1,1,1) = 64.0f;

	const float gauss_ref[27] = {
		1, 2, 1, /**/ 2, 4, 2, /**/ 1, 2, 1,
		2, 4, 2, /**/ 4, 8, 4, /**/ 2, 4, 2,
		1, 2, 1, /**/ 2, 4, 2, /**/ 1, 2, 1
	};

	vector<float> d1(3);
	d1[0] = d1[2] = 0;
	d1[1] = 4;

	const C1DSpacialKernelPluginHandler::Instance&  skp = C1DSpacialKernelPluginHandler::instance();
	auto k1 = skp.produce("gauss:w=1");
	auto k2 = skp.produce("gauss:w=1");
	auto k3 = skp.produce("gauss:w=1");

	CSeparableConvolute sp(k1,k2,k3);

	sp.fold(d1, *k1);
	cvdebug() << d1[0] << ", " << d1[1] << ", " << d1[2] << "\n";
	assert(d1[0] == 1);
	assert(d1[2] == 1);
	assert(d1[1] == 2);

	P3DImage result = sp.filter(src);

	BOOST_CHECK_EQUAL(result->get_size(), src.get_size());

	const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result);

	int j = 0;

	for (auto i = r.begin(); i != r.end(); ++i, ++j)
		BOOST_CHECK_CLOSE(*i, gauss_ref[j], 0.01);
}
