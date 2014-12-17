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

#define VSTREAM_DOMAIN "test-transfactory"
#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/3d/transformfactory.hh>

NS_MIA_USE


using namespace std;
namespace bfs=boost::filesystem;

struct HandlerTestFixture {
	HandlerTestFixture();

};

HandlerTestFixture::HandlerTestFixture()
{

	CPathNameArray kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C3DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

BOOST_FIXTURE_TEST_CASE(test_handler, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 9u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(),
			  "affine axisrot raffine rigid rotation rotbend spline translate vf ");
}

BOOST_FIXTURE_TEST_CASE(test_translate_creator, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory affine_creater = handler.produce("translate");
	P3DTransformation transform = affine_creater->create(C3DBounds(10,20,15));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(10,20,15));
}


BOOST_FIXTURE_TEST_CASE(test_affine_creator, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory affine_creater = handler.produce("affine");
	P3DTransformation transform = affine_creater->create(C3DBounds(10,20,30));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(10,20,30));
}

BOOST_FIXTURE_TEST_CASE(test_rigid_creator, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory transform_creater = handler.produce("rigid");
	P3DTransformation transform = transform_creater->create(C3DBounds(10,20,10));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(10,20,10));
}



BOOST_FIXTURE_TEST_CASE(test_spline_creator_isotropic, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory spline_creater =
		handler.produce("spline:kernel=[bspline:d=4],rate=4");
	P3DTransformation transform = spline_creater->create(C3DBounds(16,32,32));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(16,32,32));

	// +4 because we add a boundary of 2 rows/columns for a spline of degree 4
	BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((4+4) * (8+4)* (8+4) * 3));
}

BOOST_FIXTURE_TEST_CASE(test_spline_creator_anisotropic, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory spline_creater =
		handler.produce("spline:kernel=[bspline:d=4],anisorate=[<2,4,8>]");
	P3DTransformation transform = spline_creater->create(C3DBounds(16,32,32));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(16,32,32));

	// +4 because we add a boundary of 2 rows/columns for a spline of degree 4
	BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((8+4) * (8+4)* (4+4) * 3));
}

#if 0
BOOST_FIXTURE_TEST_CASE(test_vf_creator, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory affine_creater = handler.produce("vf");
	P3DTransformation transform = affine_creater->create(C3DBounds(10,20));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(10,20));
}



BOOST_FIXTURE_TEST_CASE(test_transform_creator_option, HandlerTestFixture)
{
	auto product = C3DTransformCreatorHandler::instance().produce("spline"); 
	BOOST_CHECK_EQUAL(product->get_init_string(), "spline"); 
	
	PCmdOption option = make_opt(product, "rigid", "spline", 's',"Some help", "help"); 
	option->post_set(); 
	
	BOOST_CHECK_EQUAL(product->get_init_string(), "rigid"); 
	
}

BOOST_FIXTURE_TEST_CASE(test_transform_creator_option2, HandlerTestFixture)
{
	C3DTransformCreatorHandler::ProductPtr product; 
	
	PCmdOption option = make_opt(product, "", "transform", 't',"Some help"); 
	
	option->set_value("vf"); 
	option->post_set(); 
	
	stringstream test; 
	option->write_value(test); 
	BOOST_CHECK_EQUAL(product->get_init_string(), "vf"); 
	
}
#endif
