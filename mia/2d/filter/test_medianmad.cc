/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/medianmad.hh>
#include <mia/2d/imageio.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace medianmad_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_medianmad_without_thresh )
{
	const size_t size_x = 7;
	const size_t size_y = 5;

	const int src[size_y][size_x] =
		{{ 0, 1, 2, 3, 2, 3, 5},
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 1, 2, 7, 6, 4, 2, 1},
		 { 3, 4, 4, 3, 4, 3, 2},
		 { 1, 3, 2, 4, 5, 6, 2}};

	// "hand filtered" w = 1 -> 3x3
	const int src_ref[size_y][size_x] =
 		{{ 1, 2, 2, 2, 3, 3, 3},
		 { 1, 2, 3, 3, 3, 3, 2},
		 { 2, 3, 4, 4, 3, 3, 2},
		 { 2, 3, 4, 4, 4, 3, 2},
		 { 3, 3, 3, 4, 4, 3, 2}};

	const int ref_mad[size_y][size_x] =
		{{ 1, 0, 1, 0, 0, 1, 0},
		 { 1, 1, 1, 1, 1, 1, 1},
		 { 1, 1, 1, 1, 1, 1, 0},
		 { 1, 1, 1, 1, 1, 1, 0},
		 { 0, 1, 1, 0, 1, 1, 0}};

	

	C2DBounds size(size_x, size_y);

	C2DSIImage *src_img = new C2DSIImage(size);
	C2DSIImage ref_median_img(size);
	C2DSIImage ref_mad_img(size);
	
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) {
			(*src_img)(x,y) = src[y][x];
			ref_median_img(x,y) = src_ref[y][x];
			ref_mad_img(x,y) = ref_mad[y][x];
		}

	auto medianmad = BOOST_TEST_create_from_plugin<C2DMedianMadImageFilterFactory>("medianmad:w=1,madfile=mad.@"); 
	P2DImage src_wrap(src_img);
	P2DImage res_wrap = medianmad->filter(*src_wrap);

	C2DSIImage * res_median = dynamic_cast<C2DSIImage *>(res_wrap.get());
	BOOST_REQUIRE(res_median);
	
	BOOST_CHECK_EQUAL(res_median->get_size(), src_wrap->get_size());
	BOOST_REQUIRE(res_median->get_size() == src_wrap->get_size());

		
	auto ires = res_median->begin();
	auto iref = ref_median_img.begin();
	
	for (unsigned y = 0; y < size_y; ++y) {
		for (unsigned x = 0; x < size_x; ++x, ++ires, ++iref) {
			if (fabs(*ires - *iref) > 1e-5
				) {
				cvfail() << "Error at (" << x << ", " << y << ") got " << *ires
					 << " expect "<< *iref << "\n"; 
			}
		}
	}
	
	for (auto ires = res_median->begin(), iref = ref_median_img.begin();
	     ires != res_median->end(); ++ires, ++iref) {
		BOOST_CHECK_EQUAL(*ires, *iref);
	}		

	
	P2DImage res_mad_wrap = load_image2d("mad.@");
	BOOST_REQUIRE(res_mad_wrap); 
	C2DSIImage *res_mad = dynamic_cast<C2DSIImage *>(res_mad_wrap.get());
	BOOST_REQUIRE(res_mad);
		
	BOOST_CHECK_EQUAL(res_mad->get_size(), src_wrap->get_size());
	BOOST_REQUIRE(res_mad->get_size() == src_wrap->get_size());


	ires = res_mad->begin();
	iref = ref_mad_img.begin();

	for (unsigned y = 0; y < size_y; ++y) {
		for (unsigned x = 0; x < size_x; ++x, ++ires, ++iref) {
			if (fabs(*ires - *iref) > 1e-5
				) {
				cvfail() << "MAD: Error at (" << x << ", " << y << ") got " << *ires
					 << " expect "<< *iref << "\n"; 
			}
		}
	}

	
	for (ires = res_mad->begin(), iref = ref_mad_img.begin();
	     ires != res_mad->end(); ++ires, ++iref) {
		BOOST_CHECK_EQUAL(*ires, *iref);
	}		
}
