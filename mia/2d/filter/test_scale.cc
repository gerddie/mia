/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/core/shared_ptr.hh>
#include <mia/internal/autotest.hh>
#include <mia/2d/filter/scale.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace scale_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_ngfnormimg )
{

	const short init[16] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
	};

	const short test[4] = {
		0, 1, 2, 3
	};

	C2DSSImage fimage(C2DBounds(4, 4), init );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));


	CScale scaler(C2DBounds(2,2), "bspline3");

	P2DImage scaled = scaler.filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(2, 2));

	const C2DSSImage *fscaled = dynamic_cast<const C2DSSImage *>(scaled.get());
	BOOST_REQUIRE(fscaled);
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(2, 2));

	BOOST_CHECK_EQUAL(fscaled->get_pixel_size(), C2DFVector(1.0f, 1.5f));


	BOOST_CHECK(equal(fscaled->begin(), fscaled->end(), test));

}
