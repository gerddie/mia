/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/scale.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace scale_2dimage_filter;

CSplineKernelTestPath init_path; 

BOOST_AUTO_TEST_CASE( test_downscale )
{

	const short init[16] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
	};

	const short test[4] = {
		0, 1, 2, 3
	};

	C2DSSImage fimage(C2DBounds(4, 4), init );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));


	auto  f = BOOST_TEST_create_from_plugin<C2DScaleFilterPlugin>("scale:s=[<2,2>],interp=[bspline:d=3]"); 

	P2DImage scaled = f->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(2, 2));

	const C2DSSImage& fscaled = dynamic_cast<const C2DSSImage& >(*scaled);
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(2, 2));

	BOOST_CHECK_EQUAL(fscaled.get_pixel_size(), C2DFVector(1.0f, 1.5f));

	for (size_t i = 0; i < 4; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], test[i]); 
	}
		

}

BOOST_AUTO_TEST_CASE( test_downscale_float )
{

	const float init[16] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
	};

	// it is not confirmed that this data is correct
	const float test[4] = {
		-0.304008901,
		 0.8986637,
		 2.10133624,
		 3.30400896
	};

	C2DFImage fimage(C2DBounds(4, 4), init );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));


	auto  f = BOOST_TEST_create_from_plugin<C2DScaleFilterPlugin>("scale:s=[<2,2>],interp=[bspline:d=3]"); 
	P2DImage scaled = f->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(2, 2));

	const C2DFImage& fscaled = dynamic_cast<const C2DFImage& >(*scaled);
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(2, 2));

	BOOST_CHECK_EQUAL(fscaled.get_pixel_size(), C2DFVector(1.0f, 1.5f));

	for (size_t i = 0; i < 4; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << test[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], test[i]); 
	}
		

}

BOOST_AUTO_TEST_CASE( test_noscale )
{

	const float init[16] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
	};

	C2DFImage fimage(C2DBounds(4, 4), init );
	fimage.set_pixel_size(C2DFVector(2.0, 3.0));


	auto  f = BOOST_TEST_create_from_plugin<C2DScaleFilterPlugin>("scale:s=[<0,0>],interp=[bspline:d=3]"); 

	P2DImage scaled = f->filter(fimage);

	BOOST_CHECK_EQUAL(scaled->get_size(),C2DBounds(4, 4));

	const C2DFImage& fscaled = dynamic_cast<const C2DFImage& >(*scaled);

	BOOST_CHECK_EQUAL(fscaled.get_pixel_size(), C2DFVector(2.0f, 3.0f));

	for (size_t i = 0; i < 16; ++i) {
		cvdebug() << i << ":" << fscaled[i] << " - " << init[i] << '\n'; 
		BOOST_CHECK_EQUAL(fscaled[i], init[i]); 
	}
		

}
