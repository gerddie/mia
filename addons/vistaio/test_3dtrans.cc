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
#include <vistaio/3dtrans.hh>
#include <mia/3d/transformfactory.hh>

using namespace std;
using namespace mia;
using namespace vista_3dtrans_io;

BOOST_AUTO_TEST_CASE(test_simple_io)
{
       C3DTransformationIOPluginHandler::instance();
       C3DBounds size(4, 5, 6);
       auto tf =  produce_3dtransform_factory("affine");
       auto t = tf->create(size);
       auto params = t->get_parameters();
       int i = 0;

       for (auto ip = params.begin(); ip != params.end(); ++ip) {
              *ip = i++;
       }

       t->set_parameters(params);
       t->set_attribute("string_attr", "string");
       t->set_attribute(C3DTransformation::input_spacing_attr, PAttribute(new TAttribute<C3DFVector>(C3DFVector(2, 3, 4))));
       t->set_attribute(C3DTransformation::output_spacing_attr, PAttribute(new TAttribute<C3DFVector>(C3DFVector(1.1f, 1, 2))));
       C3DVistaTransformationIO io;
       io.save("transform.v3dt", *t);
       auto t_loaded = io.load("transform.v3dt");
       auto loaded_params = t_loaded->get_parameters();
       BOOST_REQUIRE(t_loaded->degrees_of_freedom() == 12u);
       BOOST_CHECK_EQUAL(t_loaded->get_size(), size);
       i = 0;

       for (auto ip = loaded_params.begin(); ip != loaded_params.end(); ++ip) {
              BOOST_CHECK_EQUAL(*ip, i++);
       }

       BOOST_REQUIRE(t_loaded->has_attribute("string_attr"));
       BOOST_REQUIRE(t_loaded->has_attribute(C3DTransformation::output_spacing_attr));
       BOOST_REQUIRE(t_loaded->has_attribute(C3DTransformation::input_spacing_attr));
       BOOST_CHECK_EQUAL(t_loaded->get_attribute_as<string>("string_attr"), "string");
       BOOST_CHECK_EQUAL(t_loaded->get_attribute_as<C3DFVector>(C3DTransformation::input_spacing_attr), C3DFVector(2, 3, 4));
       BOOST_CHECK_EQUAL(t_loaded->get_attribute_as<C3DFVector>(C3DTransformation::output_spacing_attr), C3DFVector(1.1f, 1, 2));
}


