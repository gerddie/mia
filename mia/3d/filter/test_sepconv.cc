/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <mia/3d/filter/sepconv.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace sepconv_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_sepconv )
{
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

	P3DImage result = sp.filter(src);

	BOOST_CHECK_EQUAL(result->get_size(), src.get_size());

	const C3DFImage& r = dynamic_cast<const C3DFImage&>(*result);

	int j = 0;

	for (auto i = r.begin(); i != r.end(); ++i, ++j)
		BOOST_CHECK_CLOSE(*i, gauss_ref[j], 0.01);
}
