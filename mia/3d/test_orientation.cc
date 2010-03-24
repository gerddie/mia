/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2007 - 2010
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#define BOOST_TEST_MODULE 2DIMAGEFIFOFILTER_TEST
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK
#include <sstream>
#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/3d/orientation.hh>


NS_MIA_USE
using namespace std;
using namespace boost;

using namespace boost::unit_test;

static void run_test_read(const string& id, E3DImageOrientation test_orient)
{
	E3DImageOrientation orient;
	istringstream is(id);
	is >> orient;
	BOOST_CHECK_EQUAL(orient, test_orient);

	stringstream os;
	os << test_orient;
	BOOST_CHECK_EQUAL(os.str(), id);

}

BOOST_AUTO_TEST_CASE( test_orientation_streamio )
{
	run_test_read("axial", ior_axial);
	run_test_read("coronal", ior_coronal);
	run_test_read("saggital", ior_saggital);
	run_test_read("unknown", ior_unknown);
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

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	mia::CCmdOptionList().parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
