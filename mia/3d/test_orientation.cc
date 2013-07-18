/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define BOOST_TEST_MODULE TEST_ORIENTATION 
#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/orientation.hh>


NS_MIA_USE
using namespace std;
using namespace boost;

using namespace boost::unit_test;

static void run_test_read(const string& id, E3DImageOrientation test_orient, const string& expect)
{
	E3DImageOrientation orient;
	istringstream is(id);
	is >> orient;
	BOOST_CHECK_EQUAL(orient, test_orient);

	stringstream os;
	os << test_orient;
	BOOST_CHECK_EQUAL(os.str(), expect);

}

BOOST_AUTO_TEST_CASE( test_orientation_streamio )
{
	run_test_read("axial", ior_axial, "axial");
	run_test_read("coronal", ior_coronal, "coronal");
	run_test_read("saggital", ior_saggital, "saggital");
	run_test_read("unknown", ior_undefined, "(undefined)");
}

BOOST_AUTO_TEST_CASE( test_orientation_attribute )
{
	COrientationTranslator::register_for("orientation");
	PAttribute attr = CStringAttrTranslatorMap::instance().to_attr("orientation", "axial");
	C3DImageOrientation *io = dynamic_cast<C3DImageOrientation *>(attr.get());
	BOOST_REQUIRE(io);

	E3DImageOrientation orient = *io;
	BOOST_CHECK_EQUAL(orient, ior_axial);
}

