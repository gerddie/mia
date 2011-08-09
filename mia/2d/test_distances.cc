/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/2d/distances.hh>


NS_MIA_USE

using namespace std;
using namespace ::boost;

struct PointLineDistanceFixture {

	PointLineDistanceFixture();

	void check_point(const C2DFVector& point, float result);

	C2DFVector a;
	C2DFVector b;
};

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_end_point, PointLineDistanceFixture )
{
	check_point(a, 0.0f);
	check_point(b, 0.0f);
}


BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_line, PointLineDistanceFixture )
{
	check_point(C2DFVector(7, 10), 5.0f);
	check_point(C2DFVector(-2, -2), 5.0f);

}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_on_segment, PointLineDistanceFixture )
{
	check_point(C2DFVector(2.5, 4), 0.0f);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_a, PointLineDistanceFixture )
{
	check_point(C2DFVector(1, 0), 2);
	check_point(C2DFVector(1, 1), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_behind_b, PointLineDistanceFixture )
{
	check_point(C2DFVector(4, 7), 1);
	check_point(C2DFVector(5, 6), 1);
}

BOOST_FIXTURE_TEST_CASE( test_line_point_distance_normal, PointLineDistanceFixture )
{
	check_point(C2DFVector(1, 6), 2.4 );
}



PointLineDistanceFixture::PointLineDistanceFixture():
	a(1,2),
	b(4,6)
{
}

void PointLineDistanceFixture::check_point(const C2DFVector& point, float result)
{
	BOOST_CHECK_CLOSE(distance_point_line(point, a, b) + 1.0f, result + 1.0f, 0.1f);
}

