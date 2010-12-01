/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/factorycmdlineoption.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct HandlerTestFixture {
	HandlerTestFixture();

};

HandlerTestFixture::HandlerTestFixture()
{

	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

BOOST_FIXTURE_TEST_CASE(test_handler, HandlerTestFixture)
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 5u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(),
			  "affine rigid spline translate vf ");
}


BOOST_FIXTURE_TEST_CASE(test_affine_creator, HandlerTestFixture)
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory affine_creater = handler.produce("affine");
	P2DTransformation transform = affine_creater->create(C2DBounds(10,20));
	BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10,20));
}

BOOST_FIXTURE_TEST_CASE(test_spline_creator, HandlerTestFixture)
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory spline_creater =
		handler.produce("spline:interp=bspline4,rate=4");
	P2DTransformation transform = spline_creater->create(C2DBounds(16,32));
	BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(16,32));

	// +4 because we add a boundary of 2 rows/columns for a spline of degree 4
	BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((4+4) * (8+4) * 2));
}

BOOST_FIXTURE_TEST_CASE(test_vf_creator, HandlerTestFixture)
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory affine_creater = handler.produce("vf");
	P2DTransformation transform = affine_creater->create(C2DBounds(10,20));
	BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10,20));
}


BOOST_FIXTURE_TEST_CASE(test_transform_creator, HandlerTestFixture)
{
	const C2DTransformCreatorHandler::Instance& handler =
		C2DTransformCreatorHandler::instance();
	P2DTransformationFactory transform_creater = handler.produce("translate");
	P2DTransformation transform = transform_creater->create(C2DBounds(10,20));
	BOOST_CHECK_EQUAL(transform->get_size(), C2DBounds(10,20));
}

BOOST_FIXTURE_TEST_CASE(test_transform_creator_option, HandlerTestFixture)
{
	auto product = C2DTransformCreatorHandler::instance().produce("spline"); 
	BOOST_CHECK_EQUAL(product->get_init_string(), "spline"); 
	
	PCmdOption option = make_opt(product, "spline", 's',"Some help", "help", false); 
	
	stringstream test; 
	option->write_value(test); 
	BOOST_CHECK_EQUAL(test.str(), "=spline"); 
	
}

BOOST_FIXTURE_TEST_CASE(test_transform_creator_option2, HandlerTestFixture)
{
	C2DTransformCreatorHandler::ProductPtr product; 
	
	PCmdOption option = make_opt(product, "transform", 't',"Some help", "help", false); 
	
	option->set_value("vf"); 
	
	stringstream test; 
	option->write_value(test); 
	BOOST_CHECK_EQUAL(test.str(), "=vf"); 
	
}
