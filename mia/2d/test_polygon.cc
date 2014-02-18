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

#include <mia/internal/autotest.hh>
#include <mia/2d/polygon.hh>

NS_MIA_USE


BOOST_AUTO_TEST_CASE( test_polygon )
{
	C2DPolygon poly;

	BOOST_CHECK_EQUAL(poly.size(), 0u);
	poly.append(C2DFVector(0.0, 0.0));
	BOOST_CHECK_EQUAL(poly.size(), 1u);
	poly.append(C2DFVector(0.0, 1.0));
	BOOST_CHECK_EQUAL(poly.size(), 2u);
}

BOOST_AUTO_TEST_CASE( test_polygon_inside_closed_set )
{
	C2DPolygon poly;
	C2DFVector p0(0.0f, 0.0f);
	C2DFVector p1(0.5f, 0.0f);
	C2DFVector p2(0.5f, 0.5f);

	BOOST_CHECK(!poly.is_inside_closed_set(p1));

	poly.append(C2DFVector(0.0, 0.0));

	BOOST_CHECK(poly.is_inside_closed_set(p0));
	BOOST_CHECK(!poly.is_inside_closed_set(p1));


	poly.append(C2DFVector(1.0, 0.0));


	BOOST_CHECK(poly.is_inside_closed_set(p0));
	BOOST_CHECK(poly.is_inside_closed_set(p1));
	BOOST_CHECK(!poly.is_inside_closed_set(p2));

	poly.append(C2DFVector(0.0, 1.0));

	BOOST_CHECK(poly.is_inside_closed_set(p0));
	BOOST_CHECK(poly.is_inside_closed_set(p1));
	BOOST_CHECK(poly.is_inside_closed_set(p2));

	BOOST_CHECK(poly.is_inside_closed_set(C2DFVector(0.1,0.1)));
	BOOST_CHECK(!poly.is_inside_closed_set(C2DFVector(1,1)));

	BOOST_CHECK(!poly.is_inside_closed_set(C2DFVector(2,0)));
	BOOST_CHECK(!poly.is_inside_closed_set(C2DFVector(-0.5,0)));


}



BOOST_AUTO_TEST_CASE( test_polygon_inside_open_set )
{
	C2DPolygon poly;
	C2DFVector p0(0.0f, 0.0f);
	C2DFVector p1(0.5f, 0.0f);
	C2DFVector p2(0.5f, 0.5f);

	BOOST_CHECK(!poly.is_inside_open_set(p1));
	poly.append(p0);

	BOOST_CHECK(!poly.is_inside_open_set(p0));

	poly.append(C2DFVector(1.0, 0.0));
	BOOST_CHECK(!poly.is_inside_open_set(p1));

	poly.append(C2DFVector(0.0, 1.0));


	BOOST_CHECK(!poly.is_inside_open_set(p0));
	BOOST_CHECK(!poly.is_inside_open_set(p1));
	BOOST_CHECK(!poly.is_inside_open_set(p2));

	BOOST_CHECK(poly.is_inside_open_set(C2DFVector(0.1,0.1)));
	BOOST_CHECK(!poly.is_inside_open_set(C2DFVector(1,1)));

	BOOST_CHECK(!poly.is_inside_open_set(C2DFVector(2,0)));
	BOOST_CHECK(!poly.is_inside_open_set(C2DFVector(-0.5,0)));

}

BOOST_AUTO_TEST_CASE( test_polygon_min_distance )
{
	C2DPolygon poly;
	C2DFVector p0(0.0f, 0.0f);
	C2DFVector p1(0.5f, 0.0f);
	C2DFVector p2(0.5f, 0.5f);

	poly.append(p0);
	poly.append(p1);
	poly.append(p2);

	BOOST_CHECK_EQUAL(poly.get_mimimum_distance(p1), 0.0);
	BOOST_CHECK_EQUAL(poly.get_mimimum_distance(p2), 0.0);
	BOOST_CHECK_EQUAL(poly.get_mimimum_distance(p0), 0.0);

	BOOST_CHECK_EQUAL(poly.get_mimimum_distance(C2DFVector(-0.5, 0.0)), 0.5);
	BOOST_CHECK_CLOSE(poly.get_mimimum_distance(C2DFVector(1.0, 1.0)), sqrtf(0.5f), 0.1);
}

BOOST_AUTO_TEST_CASE( test_polygon_hausdorff_distance )
{
	C2DPolygon poly1;
	C2DPolygon poly2;
	C2DFVector p0(0.0f, 0.0f);
	C2DFVector p1(0.5f, 0.0f);
	C2DFVector p2(0.5f, 0.5f);
	C2DFVector p3(1.0f, 1.0f);

	poly1.append(p0);
	poly1.append(p1);
	poly1.append(p2);

	poly2.append(p3);
	poly2.append(p1);
	poly2.append(p2);

	BOOST_CHECK_EQUAL(poly1.get_hausdorff_distance(poly1), 0.0);
	BOOST_CHECK_CLOSE(poly1.get_hausdorff_distance(poly2), sqrtf(0.5f), 0.1);
	BOOST_CHECK_CLOSE(poly2.get_hausdorff_distance(poly1), sqrtf(0.5f), 0.1);
}


