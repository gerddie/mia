/* -*- mia-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define BOOST_TEST_MODULE FIFO_FILTER_TEST
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_DYN_LINK


#include <vector>
#include <iostream>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
//#include <boost/test/floating_point_comparison.hpp>


#include <mia/core/cmdlineparser.hh>
#include <mia/3d/3DVector.hh>
#include <mia/mesh/triangulate.hh>


NS_MIA_USE

BOOST_AUTO_TEST_CASE(  test_trianguale )
{
	std::vector<C3DFVector> vlist;

	vlist.push_back(C3DFVector(0.0, 0.0, 0.0));
	vlist.push_back(C3DFVector(0.0, 3.0, 0.0));
	vlist.push_back(C3DFVector(3.0, 3.0, 0.0));
	vlist.push_back(C3DFVector(3.0, 1.0, 0.0));
	vlist.push_back(C3DFVector(4.0, 1.0, 0.0));
	vlist.push_back(C3DFVector(4.0, 3.0, 0.0));
	vlist.push_back(C3DFVector(5.0, 3.0, 0.0));
	vlist.push_back(C3DFVector(5.0, 0.0, 0.0));
	vlist.push_back(C3DFVector(2.0, 0.0, 0.0));
	vlist.push_back(C3DFVector(2.0, 2.0, 0.0));
	vlist.push_back(C3DFVector(1.0, 2.0, 0.0));
	vlist.push_back(C3DFVector(1.0, 0.0, 0.0));

	TPolyTriangulator<std::vector<C3DFVector>,std::vector<int> >  triangulator(vlist);

	std::vector<int> polygon;

	polygon.push_back(0);
	polygon.push_back(1);
	polygon.push_back(2);
	polygon.push_back(3);
	polygon.push_back(4);
	polygon.push_back(5);
	polygon.push_back(6);
	polygon.push_back(7);
	polygon.push_back(8);
	polygon.push_back(9);
	polygon.push_back(10);
	polygon.push_back(11);

	std::vector<C3DBounds> triangles;
	triangulator.triangulate(triangles, polygon);

	for (size_t i = 0; i < triangles.size(); ++i) {
		cvdebug() << triangles[i] << std::endl;
	}

	BOOST_CHECK_EQUAL(triangles.size(), 10u);

}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	mia::CCmdOptionList(" Sysopsis: run tests").parse(argc, argv);
	return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}

