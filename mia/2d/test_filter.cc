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
#include <mia/internal/pluginsettest.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>
#include <ostream>

#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/filtertest.hh>


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

NS_MIA_USE
using namespace std;

BOOST_AUTO_TEST_CASE(test_available_filters)
{
       const C2DFilterPluginHandler::Instance& handler = C2DFilterPluginHandler::instance();
       set<string> test_data = {
              "adaptmed", "admean", "aniso", "bandpass", "binarize", "close", "combiner", "convert", "crop",
              "dilate", "distance", "downscale", "erode", "gauss", "gradnorm", "invert", "kmeans",
              "label", "labelmap", "labelscale", "load", "mask", "mean", "meanvar", "median", "medianmad", "mlv", "ngfnorm",
              "noise", "open", "pruning", "regiongrow", "sandp", "scale", "selectbig", "sepconv", "shmean",
              "sobel", "sort-label", "sws", "tee", "thinning", "thresh", "tmean", "transform", "ws"
       };
#ifdef HAVE_MAXFLOW
       test_data.insert("maxflow");
#endif
       test_available_plugins(handler, test_data);
}

BOOST_AUTO_TEST_CASE(test_run_filters)
{
       C2DBounds size(2, 2);
       const unsigned int   init_data[] = {1, 10, 100, 200};
       const unsigned short test_data[] = {2, 2, 5, 2};
       C2DUIImage *int_image = new C2DUIImage(size, init_data);
       P2DImage image(int_image);
       auto bandpass = produce_2dimage_filter("bandpass:min=1,max=150");
       const string binarize("binarize:min=100,max=200");
       auto testimg = run_filters(image, bandpass, binarize, "convert:repn=ushort,map=linear,b=2,a=3");
       auto test_image = dynamic_cast<const C2DUSImage&>(*testimg);
       BOOST_CHECK_EQUAL(test_image.get_size(), size);
       auto it = test_image.begin();
       auto et = test_image.end();
       auto id = test_data;

       while (it != et) {
              BOOST_CHECK_EQUAL(*it, *id);
              ++it;
              ++id;
       }
}


BOOST_AUTO_TEST_CASE(test_chain_filters)
{
       C2DBounds size(2, 2);
       const unsigned int   init_data[] = {1, 10, 100, 200};
       const unsigned short test_data[] = {2, 2, 5, 2};
       C2DUIImage *int_image = new C2DUIImage(size, init_data);
       P2DImage image(int_image);
       auto allfilters = produce_2dimage_filter("bandpass:min=1,max=150+binarize:min=100,max=200"
                         "+convert:repn=ushort,map=linear,b=2,a=3");
       auto testimg = allfilters->filter(image);
       BOOST_CHECK_EQUAL(testimg->get_pixel_type(), it_ushort);
       auto test_image = dynamic_cast<const C2DUSImage&>(*testimg);
       BOOST_CHECK_EQUAL(test_image.get_size(), size);
       auto it = test_image.begin();
       auto et = test_image.end();
       auto id = test_data;

       while (it != et) {
              BOOST_CHECK_EQUAL(*it, *id);
              ++it;
              ++id;
       }
}


BOOST_AUTO_TEST_CASE(test_filters_chain_push_font_and_back)
{
       C2DBounds size(2, 2);
       const unsigned int   init_data[] = {1, 10, 100, 200};
       const unsigned short test_data[] = {2, 2, 5, 2};
       C2DUIImage *int_image = new C2DUIImage(size, init_data);
       P2DImage image(int_image);
       C2DImageFilterChain chain(NULL, 0);
       BOOST_CHECK(chain.empty());
       chain.push_back("binarize:min=100,max=200");
       chain.push_front("bandpass:min=1,max=150");
       chain.push_back("convert:repn=ushort,map=linear,b=2,a=3");
       auto testimg = chain.run(image);
       BOOST_CHECK_EQUAL(testimg->get_pixel_type(), it_ushort);
       auto test_image = dynamic_cast<const C2DUSImage&>(*testimg);
       BOOST_CHECK_EQUAL(test_image.get_size(), size);
       auto it = test_image.begin();
       auto et = test_image.end();
       auto id = test_data;

       while (it != et) {
              BOOST_CHECK_EQUAL(*it, *id);
              ++it;
              ++id;
       }
}

BOOST_AUTO_TEST_CASE(test_filters_chain_init_from_vector)
{
       C2DBounds size(2, 2);
       const unsigned int   init_data[] = {1, 10, 100, 200};
       const unsigned short test_data[] = {2, 2, 5, 2};
       C2DUIImage *int_image = new C2DUIImage(size, init_data);
       P2DImage image(int_image);
       vector<string> filter_descr({"bandpass:min=1,max=150",
                                    "binarize:min=100,max=200",
                                    "convert:repn=ushort,map=linear,b=2,a=3"
                                   });
       C2DImageFilterChain chain(filter_descr);
       auto testimg = chain.run(image);
       BOOST_CHECK_EQUAL(testimg->get_pixel_type(), it_ushort);
       auto test_image = dynamic_cast<const C2DUSImage&>(*testimg);
       BOOST_CHECK_EQUAL(test_image.get_size(), size);
       auto it = test_image.begin();
       auto et = test_image.end();
       auto id = test_data;

       while (it != et) {
              BOOST_CHECK_EQUAL(*it, *id);
              ++it;
              ++id;
       }
}

BOOST_AUTO_TEST_CASE(test_filters_chain_init_from_cstring_array)
{
       C2DBounds size(2, 2);
       const unsigned int   init_data[] = {1, 10, 100, 200};
       const unsigned short test_data[] = {2, 2, 5, 2};
       C2DUIImage *int_image = new C2DUIImage(size, init_data);
       P2DImage image(int_image);
       const char *filter_descr[] = {"bandpass:min=1,max=150",
                                     "binarize:min=100,max=200",
                                     "convert:repn=ushort,map=linear,b=2,a=3"
                                    };
       C2DImageFilterChain chain(filter_descr, 3);
       auto testimg = chain.run(image);
       BOOST_CHECK_EQUAL(testimg->get_pixel_type(), it_ushort);
       auto test_image = dynamic_cast<const C2DUSImage&>(*testimg);
       BOOST_CHECK_EQUAL(test_image.get_size(), size);
       auto it = test_image.begin();
       auto et = test_image.end();
       auto id = test_data;

       while (it != et) {
              BOOST_CHECK_EQUAL(*it, *id);
              ++it;
              ++id;
       }
}


