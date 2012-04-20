/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
	test_availabe_plugins(C3DImageIOPluginHandler::instance(), expected_plugins); 
}

BOOST_AUTO_TEST_CASE( test_load_landmark ) 
{
        string filename(MIA_SOURCE_ROOT"/testdata/landmarks.lmx");

	auto list = C3DImageIOPluginHandler::instance().load(filename); 
	
	BOOST_CHECK_EQUAL(list->size(), 2u); 
	BOOST_CHECK_EQUAL(list->get_name(), "Some name"); 
	

	P3DLandmark lm = list->get("Lip Upper Left");
	BOOST_CHECK_EQUAL(lm->get_name(), "Lip Upper Left");
	BOOST_CHECK(lm->has_location()); 
	BOOST_CHECK_EQUAL(lm->get_location(), C3DFVector(1, 2, 3)); 
	BOOST_CHECK_EQUAL(lm->get_isovalue(), 74.900002f); 
	
	auto viewpoint = lm->get_view(); 
	
	BOOST_CHECK_EQUAL(viewpoint.get_location(), C3DFVector(-0.11f, 0.45f, 250.0f)); 
	BOOST_CHECK_EQUAL(viewpoint.get_rotation(), Quaternion(0.00580358, 0.636834, -0.770564, -0.0253555)); 
	BOOST_CHECK_EQUAL(viewpoint.get_zoom(), 0.350000f); 


	lm = list->get("Mouth Angle Left");
	BOOST_CHECK_EQUAL(lm->get_name(), "Mouth Angle Left");
	BOOST_CHECK(!lm->has_location()); 
	BOOST_CHECK_THROW(lm->get_location(), runtime_error);
	BOOST_CHECK_EQUAL(lm->get_isovalue(), 60.0f); 
	
	viewpoint = lm->get_view(); 
	
	BOOST_CHECK_EQUAL(viewpoint.get_location(), C3DFVector(-0.12f, 0.45f, 251.0f)); 
	BOOST_CHECK_EQUAL(viewpoint.get_rotation(), Quaternion(0.00580357, 0.636833, -0.770565, -0.0253556)); 
	BOOST_CHECK_EQUAL(viewpoint.get_zoom(), 0.360000f); 
	
	
	
	
	

	
	
}
