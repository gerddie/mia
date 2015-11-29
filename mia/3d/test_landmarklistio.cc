/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/internal/pluginsettest.hh>
#include <mia/3d/landmarklistio.hh>

NS_MIA_USE
using namespace std;

BOOST_AUTO_TEST_CASE( test_available_plugins ) 
{
	set<string> expected_plugins = {"lmx", "datapool"};
	test_availabe_plugins(C3DLandmarklistIOPluginHandler::instance(), expected_plugins); 
}

BOOST_AUTO_TEST_CASE( test_load_landmarklist_lmx ) 
{
        string filename(MIA_SOURCE_ROOT"/testdata/landmarks.lmx");

	auto list = C3DLandmarklistIOPluginHandler::instance().load(filename); 
	
	BOOST_REQUIRE(list); 
	BOOST_CHECK_EQUAL(list->size(), 2u); 
	BOOST_REQUIRE(list->size() == 2u); 
	BOOST_CHECK_EQUAL(list->get_name(), "Some name"); 
	

	P3DLandmark lm = list->get("Lip Upper Left");
	BOOST_REQUIRE(lm); 
	BOOST_CHECK_EQUAL(lm->get_name(), "Lip Upper Left");
	BOOST_CHECK(lm->has_location()); 
	BOOST_CHECK_EQUAL(lm->get_location(), C3DFVector(1, 2, 3)); 
	BOOST_CHECK_EQUAL(lm->get_isovalue(), 74.900002f); 
	
	auto viewpoint = lm->get_view(); 
	BOOST_REQUIRE(lm); 
	BOOST_CHECK_EQUAL(viewpoint.get_location(), C3DFVector(-0.11f, 0.45f, 250.0f)); 
	BOOST_CHECK_EQUAL(viewpoint.get_rotation(), Quaternion(-0.0253555, 0.00580358, 0.636834, -0.770564)); 
	BOOST_CHECK_EQUAL(viewpoint.get_zoom(), 0.350000f); 


	lm = list->get("Mouth Angle Left");
	BOOST_CHECK_EQUAL(lm->get_name(), "Mouth Angle Left");
	BOOST_CHECK(!lm->has_location()); 
	BOOST_CHECK_THROW(lm->get_location(), runtime_error);
	BOOST_CHECK_EQUAL(lm->get_isovalue(), 60.0f); 
	
	viewpoint = lm->get_view(); 
	
	BOOST_CHECK_EQUAL(viewpoint.get_location(), C3DFVector(-0.12f, 0.45f, 251.0f)); 
	BOOST_CHECK_EQUAL(viewpoint.get_rotation(), Quaternion(-0.0253556, 0.00580357, 0.636833, -0.770565)); 
	BOOST_CHECK_EQUAL(viewpoint.get_zoom(), 0.360000f); 
	
}

BOOST_AUTO_TEST_CASE( test_save_load_landmarklist_lmx ) 
{
	C3DLandmarklist list; 
	list.set_name("name"); 

	auto lm1 = make_shared<C3DLandmark>("lm1", C3DFVector(1,2,3)); 
	lm1->set_isovalue(12); 
	lm1->set_view(C3DCamera(C3DFVector(2,3,4), Quaternion(1,2,3, 0.5), 2.0)); 
	list.add(lm1); 
	
	auto lm2 = make_shared<C3DLandmark>("lm2", C3DFVector(2,2,1)); 
	lm2->set_isovalue(112); 
	lm2->set_view(C3DCamera(C3DFVector(4,1,2), Quaternion(2,1,2, 1.5), 3.0)); 
	list.add(lm2);
	
	BOOST_CHECK(C3DLandmarklistIOPluginHandler::instance().save("lmtest.lmx", list));
	
	auto loaded_list = C3DLandmarklistIOPluginHandler::instance().load("lmtest.lmx");
	
	BOOST_REQUIRE(loaded_list->size() == 2u); 
	BOOST_CHECK_EQUAL(loaded_list->get_name(), "name"); 

	auto llm1 = loaded_list->get("lm1"); 
	
	BOOST_CHECK_EQUAL(*llm1, *lm1); 

	auto llm2 = loaded_list->get("lm2"); 
	
	BOOST_CHECK_EQUAL(*llm2, *lm2);
	
}
