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
#include <mia/2d/filter/crop.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace crop_2dimage_filter;


struct STestFicture {
	STestFicture();

	C2DUBImage *src_image;
	P2DImage src_wrap;
	C2DFVector test_pixelsize; 
};

STestFicture::STestFicture():
	src_image(new C2DUBImage(C2DBounds(13,17))),
	src_wrap(src_image), 
	test_pixelsize(2,3)

{
	src_image->set_pixel_size(test_pixelsize); 
	C2DUBImage::iterator i = src_image->begin();
	for (size_t y = 0; y < src_image->get_size().y; ++y)
		for (size_t x = 0; x < src_image->get_size().x; ++x, ++i) {
			*i = (x + 1) * (y + 1);
		}

}

BOOST_FIXTURE_TEST_CASE(test_crop_identity,  STestFicture)
{
	C2DCrop identity(C2DCrop::C2DSize(0,0), C2DCrop::C2DSize(-1,-1));
	P2DImage identity_wrap = identity.filter(*src_wrap);
	const C2DUBImage *identity_image = dynamic_cast<const C2DUBImage *>(identity_wrap.get());
	BOOST_REQUIRE(identity_image);
	BOOST_CHECK(equal(src_image->begin(), src_image->end(), identity_image->begin()));
	BOOST_CHECK_EQUAL( identity_wrap->get_pixel_size(), test_pixelsize); 
}

BOOST_FIXTURE_TEST_CASE(test_crop_reduce,  STestFicture)
{
	C2DCrop::C2DSize b(4,5);
	C2DCrop::C2DSize e(11,13);
	C2DCrop::C2DSize size = e - b;

	C2DCrop smaller(b,e);
	P2DImage crop_wrap = smaller.filter(*src_wrap);
	const C2DUBImage *crop_image = dynamic_cast<const C2DUBImage *>(crop_wrap.get());

	BOOST_REQUIRE(crop_image);


	BOOST_CHECK_EQUAL(crop_image->get_size().x, (size_t)size.x);
	BOOST_CHECK_EQUAL(crop_image->get_size().y, (size_t)size.y);

	for (int y = 0; y < size.y; ++y)
		for (int x = 0; x < size.x; ++x)
			BOOST_CHECK_EQUAL((*crop_image)(x,y), (*src_image)(x+b.x, y+b.y));
	
	BOOST_CHECK_EQUAL( crop_image->get_pixel_size(), test_pixelsize); 
}

BOOST_FIXTURE_TEST_CASE(test_crop_enlarge,  STestFicture)
{
	C2DCrop::C2DSize b(-1,-2);
	C2DCrop::C2DSize e(15,20);
	C2DCrop::C2DSize size = e - b;

	C2DCrop larger(b,e);
	P2DImage crop_wrap = larger.filter(*src_wrap);
	const C2DUBImage *crop_image = dynamic_cast<const C2DUBImage *>(crop_wrap.get());

	BOOST_REQUIRE(crop_image);

	BOOST_CHECK_EQUAL(crop_image->get_size().x, (size_t)size.x);
	BOOST_CHECK_EQUAL(crop_image->get_size().y, (size_t)size.y);

	for (size_t y = 0; y < src_image->get_size().y; ++y)
		for (size_t x = 0; x < src_image->get_size().x; ++x)
			BOOST_CHECK_EQUAL((*crop_image)(x-b.x,y-b.y), (*src_image)(x, y));

	BOOST_CHECK_EQUAL( crop_image->get_pixel_size(), test_pixelsize); 
}

