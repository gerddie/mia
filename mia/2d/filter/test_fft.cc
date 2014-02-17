/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/2d/filter/fft.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace fft_2dimage_filter;

class CFFT2DDummyKernel: public CFFT2DKernel {
private:
	void do_apply(const C2DBounds& /*size*/, size_t /*realsize_x*/, fftwf_complex */*cbuffer*/) const {};
};

BOOST_AUTO_TEST_CASE( test_fft )
{
	C2DBounds size(16, 32);
	C2DFImage test_image(size);
	C2DFImage::iterator ti = test_image.begin();
	for (size_t i = 0; i < test_image.size(); ++i, ++ti)
		*ti = i;

	PFFT2DKernel k(new CFFT2DDummyKernel());

	C2DFft filter(k);

	P2DImage dummy_result = filter.filter(test_image);

	BOOST_CHECK_EQUAL(dummy_result->get_pixel_type(), it_float);

	const C2DFImage *fresult = dynamic_cast<const C2DFImage *>(dummy_result.get());
	BOOST_REQUIRE(fresult);

	BOOST_REQUIRE(test_image.get_size() == fresult->get_size());

	for (C2DFImage::const_iterator cti = test_image.begin(), cfr = fresult->begin();
	     cti != test_image.end(); ++cti, ++cfr)
		BOOST_CHECK_CLOSE(*cti, *cfr,  0.1);
}
