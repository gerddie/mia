/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/3d/filter/invert.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;

BOOST_AUTO_TEST_CASE( test_invert_float )
{
	const float src_data[4] = { 1.0f, -1.0, 0.5f, 0.25f};

	const float ref_data[4] = { -1.0f, 1.0f, -0.5f, -0.25f};

	C3DFImage src(C3DBounds(2, 2, 1), src_data);

	C3DImageInvert filter;

	P3DImage res = filter.filter(src);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);

	const C3DFImage& resi = dynamic_cast<const C3DFImage& >(*res);

	BOOST_CHECK_EQUAL(resi.get_size(), src.get_size());

	C3DFImage::const_iterator f = resi.begin();
	for (size_t i = 0; i < 4; ++i, ++f)
		BOOST_CHECK_CLOSE(*f, ref_data[i], 0.1);
}
