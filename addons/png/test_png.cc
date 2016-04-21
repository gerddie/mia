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


#include <mia/internal/autotest.hh>
#include <boost/filesystem/path.hpp>

#include <mia/core/attribute_names.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/imageio.hh>

NS_MIA_USE
using namespace std; 
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem; 


BOOST_AUTO_TEST_CASE( test_load_save_8bit )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3.png");


        auto test_image = load_image2d(filename);

	const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0),   0u);
	BOOST_CHECK_EQUAL(img(1,0),  63u);
	BOOST_CHECK_EQUAL(img(0,1), 128u);
	BOOST_CHECK_EQUAL(img(1,1), 190u);
	BOOST_CHECK_EQUAL(img(0,2), 229u);
	BOOST_CHECK_EQUAL(img(1,2), 255u);

	save_image("test_image.png", test_image);

	auto test2_image = load_image2d("test_image.png");
	
	const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u);
	BOOST_CHECK_EQUAL(img2(0,1), 128u);
	BOOST_CHECK_EQUAL(img2(1,1), 190u);
	BOOST_CHECK_EQUAL(img2(0,2), 229u);
	BOOST_CHECK_EQUAL(img2(1,2), 255u);
	
}

BOOST_AUTO_TEST_CASE( test_load_save_16bit )
{
	string filename(MIA_SOURCE_ROOT"/testdata/gray2x3-16.png");


        auto test_image = load_image2d(filename);

	const C2DUSImage& img = dynamic_cast<const C2DUSImage&>(*test_image); 
	BOOST_CHECK_EQUAL(img.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img(0,0),   0u);
	BOOST_CHECK_EQUAL(img(1,0),  63u * 256);
	BOOST_CHECK_EQUAL(img(0,1), 128u * 256);
	BOOST_CHECK_EQUAL(img(1,1), 190u * 256);
	BOOST_CHECK_EQUAL(img(0,2), 229u * 256);
	BOOST_CHECK_EQUAL(img(1,2), 255u * 256);

	save_image("test_image.png", test_image);

	auto test2_image = load_image2d("test_image.png");
	
	const C2DUSImage& img2 = dynamic_cast<const C2DUSImage&>(*test2_image); 
	BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
	BOOST_CHECK_EQUAL(img2.get_size().y, 3u);

	BOOST_CHECK_EQUAL(img2(0,0),   0u);
	BOOST_CHECK_EQUAL(img2(1,0),  63u * 256);
	BOOST_CHECK_EQUAL(img2(0,1), 128u * 256);
	BOOST_CHECK_EQUAL(img2(1,1), 190u * 256);
	BOOST_CHECK_EQUAL(img2(0,2), 229u * 256);
	BOOST_CHECK_EQUAL(img2(1,2), 255u * 256);
	
}


