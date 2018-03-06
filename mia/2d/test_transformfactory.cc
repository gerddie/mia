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

#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/2d/transformfactory.hh>


NS_MIA_USE

using namespace std;
using namespace boost::unit_test;
namespace bfs = boost::filesystem;



BOOST_AUTO_TEST_CASE(test_handler)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       BOOST_CHECK_EQUAL(handler.size(), 6u);
       BOOST_CHECK_EQUAL(handler.get_plugin_names(),
                         "affine rigid rotation spline translate vf ");
}


BOOST_AUTO_TEST_CASE(test_affine_creator)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       P2DTransformationFactory affine_creater = handler.produce("affine");
       P2DTransformation transform = affine_creater->create(C2DBounds(10, 20));
       BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10, 20));
}

BOOST_AUTO_TEST_CASE(test_spline_creator_isotropic)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       P2DTransformationFactory spline_creater =
              handler.produce("spline:kernel=[bspline:d=4],rate=4");
       P2DTransformation transform = spline_creater->create(C2DBounds(16, 32));
       BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(16, 32));
       // +4 because we add a boundary of 2 rows/columns for a spline of degree 4
       BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((4 + 4) * (8 + 4) * 2));
}

BOOST_AUTO_TEST_CASE(test_spline_creator_anisotropic)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       P2DTransformationFactory spline_creater =
              handler.produce("spline:kernel=[bspline:d=4],anisorate=[<2,4>]");
       P2DTransformation transform = spline_creater->create(C2DBounds(16, 32));
       BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(16, 32));
       // +4 because we add a boundary of 2 rows/columns for a spline of degree 4
       BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((8 + 4) * (8 + 4) * 2));
}


BOOST_AUTO_TEST_CASE(test_vf_creator)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       P2DTransformationFactory affine_creater = handler.produce("vf");
       P2DTransformation transform = affine_creater->create(C2DBounds(10, 20));
       BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10, 20));
}


BOOST_AUTO_TEST_CASE(test_transform_creator)
{
       const C2DTransformCreatorHandler::Instance& handler =
              C2DTransformCreatorHandler::instance();
       P2DTransformationFactory transform_creater = handler.produce("translate");
       P2DTransformation transform = transform_creater->create(C2DBounds(10, 20));
       BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10, 20));
}

BOOST_AUTO_TEST_CASE(test_transform_creator_option)
{
       auto product = C2DTransformCreatorHandler::instance().produce("spline");
       BOOST_CHECK_EQUAL(product->get_init_string(), "spline");
       PCmdOption option = make_opt(product, "rigid", "spline", 's', "Some help");
       option->post_set();
       BOOST_CHECK_EQUAL(product->get_init_string(), "rigid");
}

BOOST_AUTO_TEST_CASE(test_transform_creator_option2)
{
       C2DTransformCreatorHandler::ProductPtr product;
       PCmdOption option = make_opt(product, "", "transform", 't', "Some help");
       option->set_value("vf");
       option->post_set();
       BOOST_CHECK_EQUAL(product->get_init_string(), "vf");
}
