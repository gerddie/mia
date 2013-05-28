/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filter/downscale.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace downscale_2dimage_filter;
namespace bfs=::boost::filesystem;

C1DSpacialKernelPluginHandlerTestPath spacial_kernel_test_path; 
C2DFilterPluginHandlerTestPath filter_test_path; 


BOOST_AUTO_TEST_CASE( test_downscale )
{
	const short init[16] = {
		0, 0, 1, 1, /**/ 0, 0, 1, 1, /**/ 2, 2, 3, 3, /**/ 2, 2, 3, 3,
	};

	const short test[4] = {
		0, 1, 2, 3
	};

	C2DSSImage *fimage  = new C2DSSImage(C2DBounds(4, 4), init );
	P2DImage image(fimage);
	fimage->set_pixel_size(C2DFVector(2.0, 3.0));

	auto filter = BOOST_TEST_create_from_plugin<C2DDownscaleFilterPlugin>("downscale:b=[<2,2>],kernel=gauss");

	P2DImage scaled = filter->filter(*image);
	C2DSSImage *fscaled = dynamic_cast<C2DSSImage *>(&*scaled);

	BOOST_CHECK_EQUAL(scaled->get_size(), C2DBounds(2, 2));
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(2, 2));

	BOOST_REQUIRE(fscaled);

	BOOST_CHECK_EQUAL(fscaled->get_pixel_size(), C2DFVector(1.0f, 1.5f));
	BOOST_CHECK(equal(fscaled->begin(), fscaled->end(), test));

	cvdebug() << "result="; 
	for (auto i = fscaled->begin(); i != fscaled->end();++i) 
		cverb << " " << *i; 
	cverb << "\n"; 
}

BOOST_AUTO_TEST_CASE( test_non_skewedness )
{
	// this test fills the columns of the image with same values 
	// and test whether after filtering the columns are still constant (exept for the boundaries) 
	// this test checks if Bug #97 is fixed for 2D filters. 

	C2DSSImage *fimage  = new C2DSSImage(C2DBounds(1687, 20));
	auto i = fimage->begin(); 
	for (size_t y = 0; y < 20; ++y)
		for (size_t x = 0; x < 1687; ++x, ++i) {
			*i = x; 
		}

	P2DImage image(fimage);
	fimage->set_pixel_size(C2DFVector(2.0, 3.0));

	auto filter = BOOST_TEST_create_from_plugin<C2DDownscaleFilterPlugin>("downscale:b=[<2,2>],kernel=gauss");

	P2DImage scaled = filter->filter(*image);
	C2DSSImage *fscaled = dynamic_cast<C2DSSImage *>(&*scaled);

	BOOST_CHECK_EQUAL(scaled->get_size(), C2DBounds(844, 10));
	BOOST_REQUIRE(scaled->get_size() == C2DBounds(844, 10));

	BOOST_REQUIRE(fscaled);

	BOOST_CHECK_EQUAL(fscaled->get_pixel_size(), C2DFVector(1.0f, 1.5f));
	
	for (size_t x = 0; x < 844; ++x)
		for (size_t y = 3; y < 7; ++y)
			BOOST_CHECK_EQUAL((*fscaled)(x, y), (*fscaled)(x, 2)); 

}
