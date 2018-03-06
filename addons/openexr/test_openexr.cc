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
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <mia/2d/imageio.hh>
#include <mia/2d/imageiotest.hh>
#include <mia/2d/vfio.hh>

NS_MIA_USE
using namespace std;
using namespace boost;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

BOOST_AUTO_TEST_CASE(test_2dimage_plugin_handler)
{
       const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance();
       BOOST_REQUIRE(handler.size() >= 2u);
       auto plugins = handler.get_set();
       BOOST_REQUIRE(plugins.find("exr") != plugins.end());
}

BOOST_AUTO_TEST_CASE(test_2dvf_plugin_handler)
{
       const C2DVFIOPluginHandler::Instance& handler = C2DVFIOPluginHandler::instance();
       BOOST_REQUIRE(handler.size() >= 2u);
       auto plugins = handler.get_set();
       BOOST_REQUIRE(plugins.find("exr") != plugins.end());
}

BOOST_AUTO_TEST_CASE ( test_2dvfio )
{
       C2DBounds size(2, 3);
       C2DFVectorfield vf(size);
       vector<C2DFVector> test_data = {
              C2DFVector(2, 3), C2DFVector(3, 4), C2DFVector(4, 5),
              C2DFVector(5, 6), C2DFVector(6, 7), C2DFVector(8, 9)
       };
       copy(test_data.begin(), test_data.end(), vf.begin());
       C2DIOVectorfield iovf(vf);
       BOOST_REQUIRE(C2DVFIOPluginHandler::instance().save("2dvf.exr", iovf));
       auto loaded = C2DVFIOPluginHandler::instance().load("2dvf.exr");
       BOOST_REQUIRE(loaded);
       BOOST_CHECK_EQUAL(loaded->get_size(), size);
       auto i = loaded->begin();
       auto e = loaded->end();
       auto t = vf.begin();

       while (i != e) {
              BOOST_CHECK_EQUAL(*i, *t);
              ++i;
              ++t;
       }
}


BOOST_AUTO_TEST_CASE ( test_2dimageio_float )
{
       C2DBounds size(2, 3);
       C2DFImage image(size);
       vector<float> test_data = {2.1, 3.3, 3.5, 4.5, 4.6, 5.2};
       copy(test_data.begin(), test_data.end(), image.begin());
       BOOST_REQUIRE(save_image("2dimage-f.exr", image));
       auto loaded = load_image2d("2dimage-f.exr");
       BOOST_REQUIRE(loaded);
       BOOST_CHECK_EQUAL(loaded->get_size(), size);
       const C2DFImage& float_loaded = dynamic_cast<const C2DFImage&>(*loaded);
       auto i = float_loaded.begin();
       auto e = float_loaded.end();
       auto t = image.begin();

       while (i != e) {
              BOOST_CHECK_EQUAL(*i, *t);
              ++i;
              ++t;
       }
}

BOOST_AUTO_TEST_CASE ( test_2dimageio_uint )
{
       C2DBounds size(2, 3);
       C2DUIImage image(size);
       vector<float> test_data = {2, 3, 3, 4, 4, 5};
       copy(test_data.begin(), test_data.end(), image.begin());
       BOOST_REQUIRE(save_image("2dimage-ui.exr", image));
       auto loaded = load_image2d("2dimage-ui.exr");
       BOOST_REQUIRE(loaded);
       BOOST_CHECK_EQUAL(loaded->get_size(), size);
       const C2DUIImage& float_loaded = dynamic_cast<const C2DUIImage&>(*loaded);
       auto i = float_loaded.begin();
       auto e = float_loaded.end();
       auto t = image.begin();

       while (i != e) {
              BOOST_CHECK_EQUAL(*i, *t);
              ++i;
              ++t;
       }
}

