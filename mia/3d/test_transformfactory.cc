/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#define VSTREAM_DOMAIN "test-transfactory"
#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE


using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

CSplineKernelTestPath splinekernel_init_path; 

struct HandlerTestFixture {
	HandlerTestFixture();

};

HandlerTestFixture::HandlerTestFixture()
{

	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C3DTransformCreatorHandler::set_search_path(kernelsearchpath);
}

BOOST_FIXTURE_TEST_CASE(test_handler, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 4u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(),
			  "affine rigid spline translate ");
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


#if 0

BOOST_FIXTURE_TEST_CASE(test_spline_creator, HandlerTestFixture)
{
	const C3DTransformCreatorHandler::Instance& handler =
		C3DTransformCreatorHandler::instance();
	P3DTransformationFactory spline_creater =
		handler.produce("spline:interp=[bspline:d=4],rate=4");
	P3DTransformation transform = spline_creater->create(C3DBounds(16,32));
	BOOST_CHECK_EQUAL(transform->get_size(), C3DBounds(16,32));

	// +4 because we add a boundary of 2 rows/columns for a spline of degree 4
	BOOST_CHECK_EQUAL(transform->degrees_of_freedom(), static_cast<size_t>((4+4) * (8+4) * 2));
}

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
	
	PCmdOption option = make_opt(product, "spline", 's',"Some help", "help"); 
	
	stringstream test; 
	option->write_value(test); 
	BOOST_CHECK_EQUAL(test.str(), "=spline"); 
	
}

BOOST_FIXTURE_TEST_CASE(test_transform_creator_option2, HandlerTestFixture)
{
	C3DTransformCreatorHandler::ProductPtr product; 
	
	PCmdOption option = make_opt(product, "transform", 't',"Some help", "help"); 
	
	option->set_value("vf"); 
	
	stringstream test; 
	option->write_value(test); 
	BOOST_CHECK_EQUAL(test.str(), "=vf"); 
	
}
#endif
