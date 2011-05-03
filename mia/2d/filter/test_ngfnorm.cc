/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#include <mia/core/shared_ptr.hh>
#include <mia/internal/autotest.hh>
#include <mia/2d/filter/ngfnorm.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace ngfnorm_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_ngfnormimg )
{
	const size_t size = 4;
	const float src_data[size * size] = {
		1, 1, 1, 1,
		2, 3, 4, 2,
		3, 4, 2, 1,
		2, 4, 1, 2,
	};

	const float sum_norm = (2 * sqrt(2.0) + sqrt(13.0) + sqrt(18.0)) / 16.0;
	const float noise2 = (51.0f - 29.0f * 29.0f / 24.0f) / 23.0f  *  sum_norm * sum_norm;
	const float imax = sqrt( (18.0 + noise2) / 18.0);

	const float ref_data[size * size] = {
		0, 0, 0, 0,
		0, sqrt(13.0 / (13.0 + noise2)) * imax, sqrt( 2.0 / (2.0 + noise2) ) * imax, 0,
		0, sqrt( 2.0 / ( 2.0 + noise2)) * imax, 1, 0,
		0, 0, 0, 0,
	};


	C2DFImage src(C2DBounds(size, size));

	C2DFImage::iterator f = src.begin();
	for (size_t i = 0; i < size*size; ++i, ++f)
		*f = src_data[i];


	CNgfnorm filter;

	P2DImage res = filter.filter(src);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);

	const C2DFImage *resi = dynamic_cast<const C2DFImage *>(res.get());
	BOOST_REQUIRE(resi);


	BOOST_REQUIRE(resi->get_size() == src.get_size());

	C2DFImage::const_iterator r = resi->begin();
	for (size_t i = 0; i < size*size; ++i, ++r)
		BOOST_CHECK_CLOSE(*r, ref_data[i],0.1);

}
