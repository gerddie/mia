/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/2d/rgbimageio.hh>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;


BOOST_AUTO_TEST_CASE( test_load_save_8bit_gray )
{
       string filename(MIA_SOURCE_ROOT"/testdata/gray2x3.jpg");
       auto test_image = load_image2d(filename);
       const C2DUBImage& img = dynamic_cast<const C2DUBImage&>(*test_image);
       BOOST_CHECK_EQUAL(img.get_size().x, 2u);
       BOOST_CHECK_EQUAL(img.get_size().y, 3u);
       BOOST_CHECK_EQUAL(img(0, 0),   0u);
       BOOST_CHECK_EQUAL(img(1, 0),  63u);
       BOOST_CHECK_EQUAL(img(0, 1), 128u);
       BOOST_CHECK_EQUAL(img(1, 1), 190u);
       BOOST_CHECK_EQUAL(img(0, 2), 229u);
       BOOST_CHECK_EQUAL(img(1, 2), 255u);
       save_image("test_image.jpg", test_image);
       auto test2_image = load_image2d("test_image.jpg");
       const C2DUBImage& img2 = dynamic_cast<const C2DUBImage&>(*test2_image);
       BOOST_CHECK_EQUAL(img2.get_size().x, 2u);
       BOOST_CHECK_EQUAL(img2.get_size().y, 3u);
       BOOST_CHECK_EQUAL(img2(0, 0),   0u);
       BOOST_CHECK_EQUAL(img2(1, 0),  63u);
       BOOST_CHECK_EQUAL(img2(0, 1), 128u);
       BOOST_CHECK_EQUAL(img2(1, 1), 190u);
       BOOST_CHECK_EQUAL(img2(0, 2), 229u);
       BOOST_CHECK_EQUAL(img2(1, 2), 255u);
       unlink("test_image.jpg");
}


BOOST_AUTO_TEST_CASE( test_load_jpeg_8bit_rgb )
{
       // the image data is lossy
       vector<unsigned char> test_data{206, 89, 97, 72, 99, 68, 192, 205, 53,
                                       28, 31, 98, 94, 27, 203, 232, 17, 216};
       const auto& io = C2DRGBImageIOPluginPluginHandler::instance();
       auto test_image = io.load(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.jpg");
       const CRGB2DImage& img = *test_image;
       BOOST_CHECK_EQUAL(img.get_size().x, 3);
       BOOST_CHECK_EQUAL(img.get_size().y, 2);
       auto pixels = img.pixel();

       for (int i = 0; i < 18; ++i) {
              BOOST_CHECK_EQUAL(pixels[i], test_data[i]);
       }
}



BOOST_AUTO_TEST_CASE( test_save_load_8bit_rgb )
{
       C2DBounds size(255, 256);
       CRGB2DImage image(size);
       auto p = image.pixel();

       for (unsigned y = 0; y < size.y; ++y) {
              for (unsigned x = 0; x < size.x; ++x, p += 3 ) {
                     p[0] = x;
                     p[1] = y;
                     p[2] = (x + y) / 2;
              }
       }

       const auto& io = C2DRGBImageIOPluginPluginHandler::instance();
       BOOST_REQUIRE(save_image("smooth.jpg", image));
       auto test_image = io.load("smooth.jpg");
       BOOST_CHECK_EQUAL(test_image->get_size().x, 255);
       BOOST_CHECK_EQUAL(test_image->get_size().y, 256);
       p = test_image->pixel();

       for (unsigned y = 0; y < size.y; ++y) {
              for (unsigned x = 0; x < size.x; ++x, p += 3 ) {
                     unsigned z = (x + y) / 2;
                     unsigned deltax = p[0] > x ? p[0] -  x : x - p[0];
                     unsigned deltay = p[1] > y ? p[1] -  y : y - p[1];
                     unsigned deltaz = p[2] > z ? p[2] -  z : z - p[2];
                     // jpeg is lossy so we need to allow for some tolerance
                     BOOST_CHECK_SMALL(deltax, 5u);
                     BOOST_CHECK_SMALL(deltay, 5u);
                     BOOST_CHECK_SMALL(deltaz, 5u);
              }
       }
}

BOOST_AUTO_TEST_CASE( test_rejects )
{
       const auto& io = C2DRGBImageIOPluginPluginHandler::instance();
       BOOST_CHECK_THROW(io.load(MIA_SOURCE_ROOT"/testdata/gray2x3.jpg"),
                         invalid_argument);
       BOOST_CHECK_THROW(io.load(MIA_SOURCE_ROOT"/testdata/nonexistent.jpg"),
                         runtime_error);
       BOOST_CHECK_THROW(load_image2d(MIA_SOURCE_ROOT"/testdata/rgb3x2-24bit.jpg"),
                         invalid_argument);
       BOOST_CHECK_THROW(load_image2d(MIA_SOURCE_ROOT"/testdata/nonexistent.jpg"),
                         runtime_error);
}
