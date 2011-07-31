/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Evolutionary Anthropoloy
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
#include <mia/2d/filter/aniso.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace aniso_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_psi_tuckey )
{
	BOOST_CHECK_CLOSE(psi_tuckey(0.0f, 1.0f) + 1.0f, 1.0f, 0.1);
	BOOST_CHECK_CLOSE(psi_tuckey(1.0f, 1.0f) + 1.0f, 1.0f, 0.1);
	BOOST_CHECK_CLOSE(psi_tuckey(0.5f, 1.0f), 0.5f * 0.75f * 0.75f, 0.1);
}

BOOST_AUTO_TEST_CASE( test_psi_pm1 )
{
	BOOST_CHECK_CLOSE(psi_pm1(0.0f, 1.0f) + 1.0f, 1.0f, 0.1);
	BOOST_CHECK_CLOSE(psi_pm1(1.0f, 1.0f) , 2.0f / 3.0f, 0.1);

}

BOOST_AUTO_TEST_CASE( test_psi_pm2 )
{
	BOOST_CHECK_CLOSE(psi_pm2(0.0f, 1.0f) + 1.0f, 1.0f, 0.1);
	BOOST_CHECK_CLOSE(psi_pm2(1.0f, 1.0f), 1.0f * expf(-0.25), 0.1);
}


struct C2DAnisoDiffN4Fixture: public C2DAnisoDiff {
	C2DAnisoDiffN4Fixture(): C2DAnisoDiff(1, 0.1, -1, psi_test, 4) {
		const C2DBounds size(5,4);
		const float init_data[20] = {
			1, 2,  3, 5, 2,    // 1 1 2 3   // 2 2 7 3 1
			3, 4, 10, 2, 3,    // 1 6 8 1   // 3 3 7 7 2
			6, 7,  3, 9, 1,    // 1 4 6 8   // 2 4 1 8 2
			8, 3,  2, 1, 3     // 5 1 1 2
		};
		src_image = C2DFImage(size, init_data);
		create_histogramfeeder(src_image);
	};
	C2DFImage src_image;
};


BOOST_FIXTURE_TEST_CASE(test_C2DAnisoDiffN4, C2DAnisoDiffN4Fixture)
{
	BOOST_CHECK_CLOSE(estimate_MAD(src_image), 1.0f, 0.001);

	update_gamma_sigma(src_image);

	BOOST_CHECK_CLOSE(m_sigma_e, 1.4826f, 0.1);
	BOOST_CHECK_CLOSE(m_sigma, float(1.4826*sqrt(5.0f)) , 0.1);
	BOOST_CHECK_CLOSE(m_gamma, 0.25f, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_C2DAnisoDiffN4_oneStep, C2DAnisoDiffN4Fixture)
{
	const float test_data[20] = {
		1, 2,  3, 5, 2,    // 1 1 2 3   // 2 2 7 3 1
		3, 5, 11, 3, 3,    // 1 6 8 1   // 3 3 7 7 2
		6, 8,  4, 10, 1,    // 1 4 6 8   // 2 4 1 8 2
		8, 3,  2, 1, 3     // 5 1 1 2
	};


	update_gamma_sigma(src_image);

	C2DFImage dest(src_image.get_size());

	float difference = diffuse(dest, src_image);

	size_t k = 0;
	for (C2DFImage::const_iterator idest = dest.begin(), isrc = src_image.begin();
	     idest != dest.end(); ++idest, ++isrc, ++k) {
		BOOST_CHECK_CLOSE(*idest, test_data[k], 0.1);
	}

	BOOST_CHECK_CLOSE(difference, 6.0f, 0.1);

}


